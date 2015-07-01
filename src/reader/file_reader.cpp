#include "file_reader.h"
#include <algorithm>
#include <boost/filesystem.hpp>
#include <glog/logging.h>

// https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html#Diagnostic-Pragmas
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#include <opencv2/highgui/highgui.hpp>
#pragma GCC diagnostic pop

namespace sigen {
image_sequence file_reader::load(const std::string &path) {
  // ディレクトリ内に存在するファイルを列挙する
  namespace fs = boost::filesystem;
  std::vector<std::string> fnames;
  for (auto &&entry : fs::directory_iterator(path)) {
    fnames.push_back(entry.path().string());
  }
  // ファイル名で昇順に並べる
  std::sort(fnames.begin(), fnames.end());
  image_sequence ret;
  for (auto &&entry : fnames) {
    LOG(INFO) << entry;
    // 読めなかったファイル (画像以外のファイル) 以外を読み込む
    cv::Mat im = cv::imread(entry, 0 /* grayscale */);
    if (im.data) {
      ret.push_back(im);
    }
  }
  return ret;
}
};