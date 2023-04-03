#include <bits/stdc++.h>
using namespace std;
namespace fs = std::filesystem;

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
  }

  ~FileReader() {
    if (file_.is_open()) file_.close();
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
    }
  }

  std::string fileName_;
  std::ifstream file_;
  std::vector<std::string> buffer_;
  std::vector<std::string>::iterator bufferIter_;
  bool isEOF_;
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

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <file_path>" << std::endl;
    return -1;
  }
  
  {
    FileReader reader(argv[1]);
    int stringCount = 0;
    FileWriter writer("output.txt");
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

      writer.Push(str);
    }
    std::cout << "Total processed string count: " << stringCount << std::endl;
  }

  {
    FileReader reader("output.txt");
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
}