#include <bits/stdc++.h>
using namespace std;

const int64_t FLAGS_rebuild_sorted_buffer_size_MB = 2;

class FileBuffer {
public:
  FileBuffer(const std::string& dirPath) : dirPath_(dirPath) {
    if (!std::filesystem::exists(dirPath_)) {
      std::filesystem::create_directories(dirPath_);
    }

    // delete the content of the directory to make it clear
    for (const auto& entry : std::filesystem::directory_iterator(dirPath_)) {
      std::filesystem::remove_all(entry.path());
    }
  }

  void ingest(std::string& str) {
    bufferedSize_ += str.size();
    buffer_.emplace_back(std::move(str));
    if (bufferedSize_ >= (FLAGS_rebuild_sorted_buffer_size_MB << 20)) {
      flush();
    }
  }

  void flush() {
    if (buffer_.empty()) {
      return;
    }
    std::string fileName = generateFileName();
    std::ofstream outFile(dirPath_ + "/" + fileName, std::ios::binary);
    std::sort(buffer_.begin(), buffer_.end());
    for (const auto& str : buffer_) {
      uint32_t len = str.size();
      outFile.write(reinterpret_cast<const char*>(&len), sizeof(len));
      outFile.write(str.data(), len);
    }
    buffer_.clear();
    bufferedSize_ = 0;
    outFile.close();
    fileNames_.emplace_back(std::move(fileName));
  }

  std::vector<std::string> getFileNames() const {
      return fileNames_;
  }

private:
  std::string dirPath_;
  bool needSort_;
  uint64_t bufferedSize_ = 0;
  std::vector<std::string> buffer_;
  std::vector<std::string> fileNames_;
  uint64_t fileCount_ = 0;

  // TODO: build a unique file name, maybe epoch + file count 
  std::string generateFileName() {
    return std::to_string(fileCount_++);
  }
};

FILE* openFile(char* fileName, char* mode) {
	FILE* fp = fopen(fileName, mode);
	if (fp == NULL) {
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}
	return fp;
}

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

void createInitialRuns(char* input_file) {
	// For big input file
	FILE* in = openFile(input_file, "r");

  FileBuffer fb("/data/smash-data/kv-1/ouwei/test/cpp/InitialRun/tmp");

	int tmp;
	while (fscanf(in, "%d ", &tmp) != EOF) {
    // std::cout << "tmp: " << tmp << std::endl;
    int32_t encoded = EncodeInt32(tmp);
    std::string tmpStr;
    tmpStr.reserve(4);
    tmpStr.append(reinterpret_cast<char*>(&encoded), 4);
    fb.ingest(tmpStr);
	}

  fb.flush();

	fclose(in);

  auto fileNames = fb.getFileNames();
  for (const auto& fileName : fileNames) {
    std::cout << "generate file: " << fileName << std::endl;
  }
}

int main() {
  char inputFile[] = "input.txt";
  uint64_t totalSize = 0;

  FILE* in = openFile(inputFile, "w");
	srand(time(NULL));
  while (totalSize < (7 << 20)) {
    fprintf(in, "%d ", rand());
    totalSize += 4;
  }
  fclose(in);

  createInitialRuns(inputFile);
}