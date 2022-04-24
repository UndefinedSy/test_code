package main

import (
	"context"
	"database/sql"
	"flag"
	"fmt"
	"os"
	"os/signal"
	"reflect"
	"strings"
	"sync"
	"syscall"

	_ "github.com/go-sql-driver/mysql"
)

func newMySQL(addr, user, pwd string) (db *sql.DB, err error) {
	dsn := fmt.Sprintf("%s:%s@tcp(%s)/?interpolateParams=True",
		/*user:*/ user,
		/*password:*/ pwd,
		/*addr:*/ addr)

	_db, err := sql.Open("mysql", dsn)
	if err != nil {
		fmt.Printf("Failed to open the target mysql with error: %s", err)
		return
	}

	_db.SetConnMaxLifetime(555)
	_db.SetMaxOpenConns(1024)
	_db.SetMaxIdleConns(512)
	err = _db.Ping()
	if err != nil {
		fmt.Printf("Failed to connect to the target mysql with error: %s\n", err)
		return
	}

	db = _db
	return
}

const (
	select_sql            = "select * from %s where id >= %d and id < %d;"
	select_sql_with_index = "select * from %s where %s = ?;"
)

func repeatedCheckMySQL(
	ctx context.Context,
	db *sql.DB,
	tbl string,
	index string,
	sharding string,
	wg *sync.WaitGroup) {

	start_offset := 0
	step_size := 10

	for {
		select {
		case <-ctx.Done():
			wg.Done()
			return
		default:
			func() {
				conn, _ := db.Conn(ctx)
				defer conn.Close()
				boardcast_select_sql := fmt.Sprintf(select_sql, tbl, start_offset, start_offset+step_size)
				boardcast_select_sql = "/*MYHUB FOREACH_ALL_SHARD*/" + boardcast_select_sql
				rows, err := conn.QueryContext(ctx, boardcast_select_sql)
				if err != nil {
					fmt.Printf("boardcast query[%s] failed, err:%v\n",
						boardcast_select_sql, err)
					return
				}
				defer rows.Close()

				if err = rows.Err(); err != nil {
					fmt.Printf("Get error response in executing[%s] with error: %s\n",
						boardcast_select_sql, err)
					return
				}

				colTypes, err := rows.ColumnTypes()
				if err != nil {
					fmt.Printf("Failed to get column type with error: %s\n", err)
					return
				}

				index_col_idx := -1
				sharding_col_idx := -1
				for i, col := range colTypes {
					if strings.EqualFold(col.Name(), index) {
						index_col_idx = i
					}
					if strings.EqualFold(col.Name(), sharding) {
						sharding_col_idx = i
					}
				}
				if index_col_idx == -1 || sharding_col_idx == -1{
					panic("Failed to find the index/sharding column")
				}

				select_check_sql := fmt.Sprintf(select_sql_with_index, tbl, index)

				for rows.Next() {
					cols := make([]interface{}, len(colTypes))
					colPtrs := make([]interface{}, len(colTypes))
					for i := range cols {
						colPtrs[i] = &cols[i]
					}

					if err := rows.Scan(colPtrs...); err != nil {
						fmt.Printf("Failed to scan row with error: %s\n", err)
						return
					}

					if cols[index_col_idx] == nil {
						continue
					}

					index_col_val := *colPtrs[index_col_idx].(*interface{})
					switch index_col_val.(type) {
					case []byte:
						index_col_val = string(index_col_val.([]byte))
					}
					
					indexQueryRows, err := db.QueryContext(ctx, select_check_sql, index_col_val)
					if err != nil {
						fmt.Printf("query[%s] with gsi[%T|%v], return error: %s\n",
							select_check_sql,
							index_col_val, index_col_val,
							err)
						return
					}
					defer indexQueryRows.Close()

					// loop_count := 0 // debug
					// fmt.Printf("will check return rows with index_col: %v\n", index_col_val)
					for indexQueryRows.Next() {
						// loop_count++
						cols_i := make([]interface{}, len(colTypes))
						colPtrs_i := make([]interface{}, len(colTypes))
						for i := range cols_i {
							colPtrs_i[i] = &cols_i[i]
						}
						
						if err := indexQueryRows.Scan(colPtrs_i...); err != nil {
							fmt.Printf("Failed to scan index query row with error: %s\n", err)
							return
						}

						if !reflect.DeepEqual(
							*colPtrs[sharding_col_idx].(*interface{}),
							*colPtrs_i[sharding_col_idx].(*interface{})) {
							
							// fmt.Printf("origin: [%d][%v]\n", loop_count, *colPtrs[sharding_col_idx].(*interface{}))
							// fmt.Printf("index: [%d][%v]\n", loop_count, *colPtrs_i[sharding_col_idx].(*interface{}))
							continue
						}

						match := true
						fmt.Printf("Will check entry with index[%v]\n", index_col_val)
						for i := range cols_i {
							val_origin := *colPtrs[i].(*interface{})
							val_index := *colPtrs_i[i].(*interface{})

							if !reflect.DeepEqual(val_origin, val_index) {
								match = false
								break
							}
						}

						if !match {
							fmt.Println("------------------------------------------------------")
							fmt.Printf("Find mismatched record: [origin][index]\n")
							for _, col_origin := range colPtrs {
								var_origin := *col_origin.(*interface{})
								switch var_origin.(type) {
								case []byte:
									var_origin = string(var_origin.([]byte))
								}
								fmt.Printf("%v\t", var_origin)
							}
							fmt.Println()
							for _, col_index := range colPtrs_i {
								var_index := *col_index.(*interface{})
								switch var_index.(type) {
								case []byte:
									var_index = string(var_index.([]byte))
								}
								fmt.Printf("%v\t", var_index)
							}
							fmt.Println()
							fmt.Println("======================================================")
						}
					}

					// match := true
					// for i := range cols_i {
					// 	val_origin := *colPtrs[i].(*interface{})
					// 	val_index := *colPtrs_i[i].(*interface{})

					// 	if !reflect.DeepEqual(val_origin, val_index) {
					// 		match = false
					// 		break
					// 	}
					// }

					// if !match {
					// 	fmt.Println("------------------------------------------------------")
					// 	fmt.Printf("Find mismatched record: [origin][index]\n")
					// 	for _, col_origin := range colPtrs {
					// 		var_origin := *col_origin.(*interface{})
					// 		switch var_origin.(type) {
					// 		case []byte:
					// 			var_origin = string(var_origin.([]byte))
					// 		}
					// 		fmt.Printf("%v\t", var_origin)
					// 	}
					// 	fmt.Println()
					// 	for _, col_index := range colPtrs_i {
					// 		var_index := *col_index.(*interface{})
					// 		switch var_index.(type) {
					// 		case []byte:
					// 			var_index = string(var_index.([]byte))
					// 		}
					// 		fmt.Printf("%v\t", var_index)
					// 	}
					// 	fmt.Println()
					// 	fmt.Println("======================================================")
					// }

				}

			}()
		}

		start_offset += step_size
	}
}

var host = flag.String("h", "", "MySQL Server Hostname")
var port = flag.Int("P", 0, "MySQL Server Port")
var user = flag.String("u", "", "User name to the MySQL Server")
var pwd = flag.String("p", "", "User password to the MySQL Server")
var schema = flag.String("s", "", "schema name")
var table = flag.String("t", "", "table name")
var index = flag.String("i", "", "index name")
var sharding_key = flag.String("k", "", "sharding key")

func main() {
	flag.Parse()
	addr := fmt.Sprintf("%s:%d", *host, *port)

	db, err := newMySQL(addr, *user, *pwd)
	if err != nil {
		fmt.Printf("init db failed, err:%v\n", err)
		return
	}
	defer db.Close()

	tbl := fmt.Sprintf("`%s`.`%s`", *schema, *table)
	ctx, cancel := context.WithCancel(context.Background())
	wg := sync.WaitGroup{}
	wg.Add(1)
	go repeatedCheckMySQL(ctx, db, tbl, *index, *sharding_key, &wg)

	c := make(chan os.Signal, 1)
	done := make(chan bool, 1)
	signal.Notify(c, os.Interrupt, syscall.SIGINT, syscall.SIGTERM, syscall.SIGQUIT, syscall.SIGSEGV)
	go func() {
		<-c
		cancel()
		done <- true
	}()

	<-done
	wg.Wait()
}
