#include <bits/stdc++.h>
#include <tuple>

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
      std::cout << "Load buffer in Pop" << std::endl;
      LoadBuffer();
      return Pop();
    }
    auto& str = *bufferIter_;
    ++bufferIter_;
    ++dataPopCount_;
    return {true, std::move(str)};
  }

  std::pair<bool, std::string> Front() {
    if (buffer_.end() == bufferIter_) {
      if (isEOF_) {
        return {false, ""};
      }
      std::cout << "Load buffer in Front" << std::endl;
      LoadBuffer();
      return Front();
    }
    auto& str = *bufferIter_;
    return {true, str};
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

class LoserTree {
 public:
	LoserTree(int k, const std::vector<std::string>& inputs)
    : k_(k), inputFiles_(inputs)
  {}

	~LoserTree() {}

  bool Init(size_t inFileIdx) {
    if (inFileIdx >= inputFiles_.size()) {
      // unlikely
      std::cout << "Failed to init loser tree since out of bound" << std::endl;
      return false;
    }

    kFiles_ = std::min(k_, inputFiles_.size() - inFileIdx);
    losers_.resize(kFiles_);
    kWays_.resize(kFiles_);
    for (int i = 0; i < kFiles_; i++) {
      losers_[i] = kMin_;
      kWays_[i] = new FileReader(inputFiles_[inFileIdx + i]);
    }

    for (int i = kFiles_ - 1; i >= 0; i--) {
      Adjust(i);
    }

    auto now = time(nullptr);
    static int fileCount = 0;
    std::string output_file = "merge_" + std::to_string(now) + "_" +
                              std::to_string(fileCount++);
    out_ = std::make_unique<FileWriter>(output_file);
    outputFiles_.emplace_back(output_file);
    return true;
  }

  bool Merge() {
    while (losers_[0] != kMax_) {
      auto [ok, str] = kWays_[losers_[0]]->Pop();
      if (!ok) {
        // unlikely
        std::cout << "Failed to pop from loser tree winner" << std::endl;
        return false;
      }
      out_->Push(str);
      ++totalCount_;
      Adjust(losers_[0]);
    }
    return true;
  }

  std::vector<std::string> getOutputFiles() {
    return outputFiles_;
  }

 private:
  bool Adjust(int cur) {
    int parent = (cur + kFiles_) >> 1;
    int next = cur;

    bool hasElement;
    std::tie(hasElement, std::ignore) = kWays_[cur]->Front();
    if (!hasElement) {
      next = kMax_;
    }

    while (parent > 0) {
      if (losers_[parent] == kMin_) {
        // parent not initialized
        losers_[parent] = next;
        next = kMin_;
      } else if (next == kMax_) {
        // cur has no more element
        next = losers_[parent];
        losers_[parent] = kMax_;
      } else if (losers_[parent] == kMax_ || next == kMin_) {
        // do nothing
      } else {
        auto [ok1, str1] = kWays_[next]->Front();
        if (!ok1) {
          // unlikely, should be handled in the 2nd if
          std::cout << "Failed to get front element from loser tree" << std::endl;
          return false;
        }
        auto [ok2, str2] = kWays_[losers_[parent]]->Front();
        if (!ok2) {
          // unlikely, should be handled in the 3rd if
          std::cout << "Failed to get front element from loser tree" << std::endl;
          return false;
        }
        if (str1 > str2) {
          auto curLoser = next;
          next = losers_[parent];
          losers_[parent] = curLoser;
        }
      }

      parent >>= 1;
    }

    losers_[parent] = next;
    return true;
  }

 private:
  static const int kMin_ = -1;
  static const int kMax_ = -2;
  size_t k_;
  size_t kFiles_;
  std::vector<std::string> inputFiles_;
  std::vector<int> losers_;
  std::vector<FileReader*> kWays_;
  const std::string kMaxKey_;
  std::unique_ptr<FileWriter> out_;
  std::vector<std::string> outputFiles_;
  uint64_t totalCount_;
};

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <dir_path>" << std::endl;
    return -1;
  }
  
  std::vector<std::string> input_files;
  for (const auto& entry : std::filesystem::directory_iterator(argv[1])) {
    input_files.emplace_back(entry.path());
  }

  while (input_files.size() > 1) {
    int inputFileIdx = 0;
    LoserTree tree(3, input_files);
    while (inputFileIdx < input_files.size()) {
      if (!tree.Init(inputFileIdx)) {
        std::cout << "Failed to init loser tree" << std::endl;
        exit(-1);
      }
      if (!tree.Merge()) {
        std::cout << "Failed to merge loser tree" << std::endl;
        exit(-1);
      }
      inputFileIdx += 3;
    }
    input_files = tree.getOutputFiles();
  }

  auto finalOutputFile = input_files[0];

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
    std::cout << data << std::endl;
  }
  std::cout << "Total processed string count: " << stringCount << std::endl;
}