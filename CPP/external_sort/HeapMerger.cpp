#include <bits/stdc++.h>
using namespace std;
namespace fs = std::filesystem;

// uint64_t max in binary as the kEndOfFilePlaceHolder
std::string kEndOfFilePlaceHolderStr;

class FileReader {
public:
  FileReader(const std::string& filename)
      : fileName_(filename),
        isEOF_(false),
        bufferIter_(buffer_.end()) {
    file_.open(fileName_, std::ifstream::binary);
    if (!file_.is_open()) {
      throw std::runtime_error("Failed to open file");
    }
    file_.seekg(0, std::ios::beg);
    std::cout << "FileReader on: " << fileName_ << std::endl;
  }

  ~FileReader() {
    if (file_.is_open()) file_.close();
    std::cout << "FileReader on: " << fileName_ << " finished. "
              << "dataLoadCount: " << dataLoadCount_
              << " dataPopCount: " << dataPopCount_ << std::endl;
  }

  std::pair<bool, std::string> Pop() {
    if (buffer_.end() == bufferIter_) {
      if (isEOF_) {
        return {false, ""};
      }
      std::cout << "Load buffer" << std::endl;
      LoadBuffer();
      return Pop();
    }
    auto& str = *bufferIter_;
    ++bufferIter_;
    ++dataPopCount_;
    return {true, std::move(str)};
  }

private:
  static constexpr size_t kLoadSize = 1024;

  void LoadBuffer() {
    buffer_.clear();
    bufferIter_ = buffer_.begin();
    uint64_t loadedSize = 0;
    while (loadedSize < kLoadSize) {
      uint32_t length;
      if (!file_.read(reinterpret_cast<char*>(&length), sizeof(length))) {
        isEOF_ = true;
        buffer_.shrink_to_fit();
        bufferIter_ = buffer_.begin();
        std::cout << "error: only " << file_.gcount() << " bytes could be read\n";
        return;
      }
      std::string str(length, '\0');
      if (!file_.read(str.data(), length)) {
        // unlikely
        std::cout << "Failed to read data in file: " << fileName_ << std::endl;
        exit(-1);
      }
      buffer_.emplace_back(std::move(str));
      ++dataLoadCount_;
    }
  }

  std::string fileName_;
  std::ifstream file_;
  std::vector<std::string> buffer_;
  std::vector<std::string>::iterator bufferIter_;
  bool isEOF_;
  uint64_t dataLoadCount_ = 0;
  uint64_t dataPopCount_ = 0;
};

class FileWriter {
public:
  FileWriter(const std::string& filename)
      : fileName_(filename),
        file_(fileName_, std::ios::binary) {
    if (!file_.is_open()) {
      throw std::runtime_error("Failed to open file");
    }
  }

  ~FileWriter() {
    if (file_.is_open()) file_.close();
  }

  void Push(const std::string& str) {
    uint32_t length = str.size();
    file_.write(reinterpret_cast<const char*>(&length), sizeof(length));
    file_.write(str.data(), length);
  }

private:
  std::string fileName_;
  std::ofstream file_;
};

int32_t Bigendian(int32_t x) {
  int32_t y = x;
  char* p = reinterpret_cast<char*>(&y);
  std::reverse(p, p + 4);
  return y;
}

int32_t EncodeInt32(int32_t x) {
  static const uint32_t kInt32SignFlipBit = 1U << 31;
  x ^= kInt32SignFlipBit;
  return Bigendian(x);
}

int32_t DecodeInt32(int32_t x) {
  static const uint32_t kInt32SignFlipBit = 1U << 31;
  x = Bigendian(x);
  x ^= kInt32SignFlipBit;
  return x;
}

struct MinHeapNode {
	
	// The element to be stored
	std::string element;

	// index of the array from which
	// the element is taken
	int i;
};

// Prototype of a utility function
// to swap two min heap nodes
void swap(MinHeapNode* x, MinHeapNode* y);

// A class for Min Heap
class MinHeap {
	
	// pointer to array of elements in heap
	MinHeapNode* harr;

	// size of min heap
	int heap_size;

public:
	
	// Constructor: creates a min
	// heap of given size
	MinHeap(MinHeapNode a[], int size);

	// to heapify a subtree with
	// root at given index
	void MinHeapify(int);

	// to get index of left child
	// of node at index i
	int left(int i) { return (2 * i + 1); }

	// to get index of right child
	// of node at index i
	int right(int i) { return (2 * i + 2); }

	// to get the root
	MinHeapNode getMin() { return harr[0]; }

	// to replace root with new node
	// x and heapify() new root
	void replaceMin(MinHeapNode x)
	{
		harr[0] = x;
		MinHeapify(0);
	}
};

// Constructor: Builds a heap from
// a given array a[] of given size
MinHeap::MinHeap(MinHeapNode a[], int size)
{
	heap_size = size;
	harr = a; // store address of array
	int i = (heap_size - 1) / 2;
	while (i >= 0) {
		MinHeapify(i);
		i--;
	}
}

// A recursive method to heapify
// a subtree with root
// at given index. This method
// assumes that the
// subtrees are already heapified
void MinHeap::MinHeapify(int i)
{
	int l = left(i);
	int r = right(i);
	int smallest = i;
	
	if (l < heap_size && harr[l].element < harr[i].element)
		smallest = l;
	
	if (r < heap_size
		&& harr[r].element < harr[smallest].element)
		smallest = r;
	
	if (smallest != i) {
		swap(&harr[i], &harr[smallest]);
		MinHeapify(smallest);
	}
}

// A utility function to swap two elements
void swap(MinHeapNode* x, MinHeapNode* y)
{
	MinHeapNode temp = *x;
	*x = *y;
	*y = temp;
}

std::vector<std::string>
heapMergeFiles(int k, const std::vector<std::string>& input_files) {
  std::vector<std::string> outputFiles;

  std::vector<FileReader*> in;
  int inFileIdx = 0;
  while (inFileIdx < input_files.size()) {
    auto kFiles = std::min(k, (int)input_files.size() - inFileIdx);
    for (int i = 0; i < kFiles; i++) {
      in.emplace_back(new FileReader(input_files[inFileIdx + i]));
    }
    inFileIdx += kFiles;

    auto now = time(nullptr);
    static int fileCount = 0;
    std::string output_file = "merge_" + std::to_string(now) + "_" +
                              std::to_string(fileCount++);
    FileWriter out(output_file);
    outputFiles.emplace_back(output_file);
    
    // Create a min heap with k heap
    // nodes. Every heap node
    // has first element of scratch
    // output file
    MinHeapNode* harr = new MinHeapNode[kFiles];
    int i;
    for (i = 0; i < kFiles; i++) {
      auto [ok, str] = in[i]->Pop();
      if (!ok) {
        harr[i].element = kEndOfFilePlaceHolderStr;
      } else {
        harr[i].element = str;
      }

      // Index of scratch output file
      harr[i].i = i;
    }

    // Create the heap
    MinHeap hp(harr, i);

    int count = 0;

    // Now one by one get the
    // minimum element from min
    // heap and replace it with
    // next element.
    // run till all filled input
    // files reach EOF
    while (count != i) {
      // Get the minimum element
      // and store it in output file
      MinHeapNode root = hp.getMin();
      out.Push(root.element);

      auto [ok, str] = in[root.i]->Pop();
      if (!ok) {
        root.element = kEndOfFilePlaceHolderStr;
        count++;
        std::cout << "File " << root.i << " is done. current count: "
                  << count << " current i: " << i << std::endl;
      } else {
        root.element = str;
      }

      // Replace root with next
      // element of input file
      hp.replaceMin(root);
    }

    for (auto iPtr : in) {
      delete iPtr;
    }
    in.clear();
  }

  return outputFiles;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <dir_path>" << std::endl;
    return -1;
  }

  uint64_t kEndOfFilePlaceHolder = 0xffffffffffffffff;
  std::string tmpStr;
  tmpStr.reserve(sizeof(uint64_t));
  tmpStr.append(reinterpret_cast<char*>(&kEndOfFilePlaceHolder), 8);
  kEndOfFilePlaceHolderStr.reserve(sizeof(uint64_t));
  // kEndOfFilePlaceHolderStr.append(reinterpret_cast<char*>(
  //   &kEndOfFilePlaceHolder, sizeof(uint64_t)));
  kEndOfFilePlaceHolderStr.append(tmpStr);
  
  std::vector<std::string> input_files;
  for (const auto& entry : std::filesystem::directory_iterator(argv[1])) {
    input_files.emplace_back(entry.path());
  }

  auto outputFiles = heapMergeFiles(3, input_files);
  while (outputFiles.size() > 1) {
    outputFiles = heapMergeFiles(3, outputFiles);
  }

  auto finalOutputFile = outputFiles[0];

  FileReader reader(finalOutputFile);
  int stringCount = 0;
  while (true) {
    auto [ok, str] = reader.Pop();
    if (!ok) {
      break;
    }
    ++stringCount;
    int32_t data;
    if (str.size() != sizeof(data)) {
      std::cout << "Invalid data size: " << str.size() << std::endl;
      exit(-1);
    }
    std::memcpy(&data, str.data(), sizeof(data));
    data = DecodeInt32(data);
    // std::cout << data << std::endl;
  }
  std::cout << "Total processed string count: " << stringCount << std::endl;
}