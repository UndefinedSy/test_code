package main

import (
	"context"
	"database/sql"
	"flag"
	"fmt"
	"math/rand"
	"os"
	"os/signal"
	"sync"
	"syscall"
	"time"

	_ "github.com/go-sql-driver/mysql"
)

func newMySQL(addr, user, pwd string) (db *sql.DB, err error) {
	dsn := fmt.Sprintf("%s:%s@tcp(%s)/%s?interpolateParams=True",
		/*user:*/ user,
		/*password:*/ pwd,
		/*addr:*/ addr,
		/*dbName:*/ "test")

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
	random_select_sql = "select id from `test` limit 1;"

	update_sql = "update `test`.`test` SET data = '?' where id = 114"
	insert_sql = "insert into `test` (id) values (?)"
	delete_sql = "delete from `test` where id=?"
)

func repeatedConnectMySQL(ctx context.Context, db *sql.DB, wg *sync.WaitGroup) {
	_random_select_sql := random_select_sql

	for {
		go func() {
			select {
			case <-ctx.Done():
				wg.Done()
				return
			default:
				func() {
					conn, _ := db.Conn(ctx)
					defer conn.Close()
					r := conn.QueryRowContext(ctx, _random_select_sql)
					var _id int
					err := r.Scan(&_id)
					curTime := time.Now()
					fmt.Printf("[%v] ", curTime)
					switch {
					case err == sql.ErrNoRows:
						err = nil
						fmt.Println("Empty row returned.")
					case err != nil:
						fmt.Printf("Failed to query a rand id with error: %s\n", err)
					default:
						fmt.Println("No error returned.")
					}
				}()
			}
		}()

		time.Sleep(50 * time.Millisecond)
	}
}

func repeatedUpdateMySQL(ctx context.Context, db *sql.DB, wg *sync.WaitGroup) {
	_insert_sql := insert_sql
	_delete_sql := delete_sql

	for {
		go func() {
			select {
			case <-ctx.Done():
				wg.Done()
				return
			default:
				func() {
					conn, _ := db.Conn(ctx)
					defer conn.Close()
					rand_id := rand.Intn(2048)
					_, err := conn.ExecContext(ctx, _insert_sql, rand_id)
					_, err := conn.ExecContext(ctx, _delete_sql, rand_id)
					curTime := time.Now()
					fmt.Printf("[%v] ", curTime)
					if err != nil {
						fmt.Printf("Failed to excute update statment with error: %s\n", err)
					} else {
						fmt.Println("writer no error returned.")
					}
				}()
			}
		}()

		time.Sleep(100 * time.Millisecond)
	}
}

var host = flag.String("h", "", "MySQL Server Hostname")
var port = flag.Int("P", 5901, "MySQL Server Port")
var user = flag.String("u", "admin", "User name to the MySQL Server")
var pwd = flag.String("p", "admin", "User password to the MySQL Server")
var reader = flag.Int("r", 10, "reader coroutine")
var writer = flag.Int("w", 1, "writer coroutine")

func main() {
	flag.Parse()
	addr := fmt.Sprintf("%s:%d", *host, *port)

	db, err := newMySQL(addr, *user, *pwd)
	if err != nil {
		fmt.Printf("init db failed, err:%v\n", err)
		return
	}
	defer db.Close()

	ctx, cancel := context.WithCancel(context.Background())
	wg := sync.WaitGroup{}
	for i := 0; i < *reader; i++ {
		wg.Add(1)
		go repeatedConnectMySQL(ctx, db, &wg)
	}

	for i := 0; i < *writer; i++ {
		wg.Add(1)
		go repeatedUpdateMySQL(ctx, db, &wg)
	}

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
