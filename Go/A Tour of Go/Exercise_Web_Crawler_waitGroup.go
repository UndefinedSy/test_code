package main

import (
	"fmt"
	"sync"
)

type urlsCache struct {
	mux   sync.Mutex
	urls map[string]bool
}

func (cache *urlsCache) setVisited(name string) bool {
	cache.mux.Lock()
	defer cache.mux.Unlock()

	if cache.urls[name] {	// this url is in map, skip
		return true
	}
	cache.urls[name] = true	// mark it as used.
	return false	// this url has never been searched, will recursively call crawler
}

var cacheInstance = urlsCache{urls: make(map[string]bool)}

// Fetcher interface
type Fetcher interface {
	// Fetch returns the body of URL and
	// a slice of URLs found on that page.
	Fetch(url string) (body string, urls []string, err error)
}

func crawlInner(url string, depth int, fetcher Fetcher, wg *sync.WaitGroup) {
	defer wg.Done()
	if depth <= 0 {
		return
	}

	if cacheInstance.setVisited(url) {	// this url is in map, skip
		return
	}

	body, urls, err := fetcher.Fetch(url)
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Printf("found: %s %q\n", url, body)
	for _, u := range urls {
		wg.Add(1)
		go crawlInner(u, depth-1, fetcher, wg)
	}
	return
}

// Crawl uses fetcher to recursively crawl
// pages starting with url, to a maximum of depth.
func Crawl(url string, depth int, fetcher Fetcher) {
	waitGroup := &sync.WaitGroup{}

	waitGroup.Add(1)

	go crawlInner(url, depth, fetcher, waitGroup)

	waitGroup.Wait()
}

func main() {
	Crawl("https://golang.org/", 4, fetcher)
}

// fakeFetcher is Fetcher that returns canned results.
type fakeFetcher map[string]*fakeResult

type fakeResult struct {
	body string
	urls []string
}

func (f fakeFetcher) Fetch(url string) (string, []string, error) {
	if res, ok := f[url]; ok {
		return res.body, res.urls, nil
	}
	return "", nil, fmt.Errorf("not found: %s", url)
}

// fetcher is a populated fakeFetcher.
var fetcher = fakeFetcher{
	"https://golang.org/": &fakeResult{
		"The Go Programming Language",
		[]string{
			"https://golang.org/pkg/",
			"https://golang.org/cmd/",
		},
	},
	"https://golang.org/pkg/": &fakeResult{
		"Packages",
		[]string{
			"https://golang.org/",
			"https://golang.org/cmd/",
			"https://golang.org/pkg/fmt/",
			"https://golang.org/pkg/os/",
		},
	},
	"https://golang.org/pkg/fmt/": &fakeResult{
		"Package fmt",
		[]string{
			"https://golang.org/",
			"https://golang.org/pkg/",
		},
	},
	"https://golang.org/pkg/os/": &fakeResult{
		"Package os",
		[]string{
			"https://golang.org/",
			"https://golang.org/pkg/",
		},
	},
}