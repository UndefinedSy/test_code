package priorityqueue

import (
	"container/heap"
	"sort"
	"sync"
)

// An Item is something we manage in a priority queue.
type Item struct {
	Key      interface{} //The unique key of the item.
	Value    interface{} // The value of the item; arbitrary.
	Priority int         // The priority of the item in the queue.
	// The index is needed by update and is maintained by the heap.Interface methods.
	Index int // The index of the item in the heap.(Requeired by fix)
}

type ItemSlice struct {
	items    []*Item
	itemsMap map[interface{}]*Item
}

func (s ItemSlice) Len() int { return len(s.items) }

func (s ItemSlice) Less(i, j int) bool {
	return s.items[i].Priority < s.items[j].Priority
}

func (s ItemSlice) Swap(i, j int) {
	s.items[i], s.items[j] = s.items[j], s.items[i]
	s.items[i].Index = i
	s.items[j].Index = j
	if s.itemsMap != nil {
		s.itemsMap[s.items[i].Key] = s.items[i]
		s.itemsMap[s.items[j].Key] = s.items[j]
	}

}

func (s *ItemSlice) Push(x interface{}) {
	n := len(s.items)
	item := x.(*Item)
	item.Index = n
	s.items = append(s.items, item)
	s.itemsMap[item.Key] = item
}

func (s *ItemSlice) Pop() interface{} {
	old := s.items
	n := len(old)
	item := old[n-1]
	item.Index = -1 // for safety
	delete(s.itemsMap, item.Key)
	s.items = old[0 : n-1]
	return item
}

// update modifies the priority and value of an Item in the queue.
func (s *ItemSlice) update(key interface{}, value interface{}, priority int) {
	item := s.itemByKey(key)
	if item != nil {
		s.updateItem(item, value, priority)
	}
}

// update modifies the priority and value of an Item in the queue.
func (s *ItemSlice) updateItem(item *Item, value interface{}, priority int) {
	item.Value = value
	item.Priority = priority
	heap.Fix(s, item.Index)
}

func (s *ItemSlice) itemByKey(key interface{}) *Item {
	if item, found := s.itemsMap[key]; found {
		return item
	}
	return nil
}

// A PriorityQueue implements heap.Interface and holds Items.
type PriorityQueue struct {
	slice   ItemSlice
	maxSize int
	mutex   sync.RWMutex
}

func (pq *PriorityQueue) Init(maxSize int) {
	pq.slice.items = make([]*Item, 0, pq.maxSize)
	pq.slice.itemsMap = make(map[interface{}]*Item)
	pq.maxSize = maxSize
}

func (pq PriorityQueue) Len() int {
	pq.mutex.RLock()
	size := pq.slice.Len()
	pq.mutex.RUnlock()
	return size
}

func (pq *PriorityQueue) minItem() *Item {
	len := pq.slice.Len()
	if len == 0 {
		return nil
	}
	return pq.slice.items[0]
}

func (pq *PriorityQueue) MinItem() *Item {
	pq.mutex.RLock()
	defer pq.mutex.RUnlock()
	return pq.minItem()
}

func (pq *PriorityQueue) PushItem(key, value interface{}, priority int) (bPushed bool) {
	pq.mutex.Lock()
	defer pq.mutex.Unlock()
	size := pq.slice.Len()
	item := pq.slice.itemByKey(key)
	if size > 0 && item != nil {
		pq.slice.updateItem(item, value, priority)
		return true
	}
	item = &Item{
		Value:    value,
		Key:      key,
		Priority: priority,
		Index:    -1,
	}
	if pq.maxSize <= 0 || size < pq.maxSize {
		heap.Push(&(pq.slice), item)
		return true
	}
	min := pq.minItem()
	if min.Priority >= priority {
		return false
	}
	heap.Pop(&(pq.slice))
	heap.Push(&(pq.slice), item)
	return true
}

func (pq PriorityQueue) GetQueue() []interface{} {
	items := pq.GetQueueItems()
	values := make([]interface{}, len(items))
	for i := 0; i < len(items); i++ {
		values[i] = items[i].Value
	}
	return values
}

func (pq PriorityQueue) GetQueueItems() []*Item {
	size := pq.Len()
	if size == 0 {
		return []*Item{}
	}
	s := ItemSlice{}
	s.items = make([]*Item, size)
	pq.mutex.RLock()
	for i := 0; i < size; i++ {
		s.items[i] = &Item{
			Value:    pq.slice.items[i].Value,
			Priority: pq.slice.items[i].Priority,
		}
	}
	pq.mutex.RUnlock()
	sort.Sort(sort.Reverse(s))
	return s.items
}
