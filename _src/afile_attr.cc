#include "afile_attr.h"
#include <array>
namespace atug2 {
#define INVALID_FILE_PTR -1

  // no member functions
  const astring GetCurrentDir() {
    std::array<Wchar, MAXPATH> dir{ 0, };
    _wgetcwd(dir.data(), MAXPATH);
    return dir.data();
  }
  const string GetCurrentDirA() {
    std::array<char, MAXPATH> dir{ 0, };
    _getcwd(dir.data(), MAXPATH);
    return dir.data();
  }

  ATUG2_API const astring GetFilenameFromFullpath(const astring& _fullpath) {
    astring filename_only{ _fullpath };
    const size_t div_pos{ filename_only.find_last_of(DIV) };
    if (astring::npos NE div_pos) {
      return filename_only.replace(0, div_pos + 1, L"");
    } else {
      return EMPTY_STR;
    }
  }

  astring PathStr::GetFullpath() const {
    astring fullpath{};
    fullpath += GetDir() + DIV + GetName() + DOT + GetExtension();
    return fullpath.c_str();
  }
  astring PathStr::GetDir() const {
    return dir_.GetWstr();
  }
  astring PathStr::GetName() const {
    return name_.GetWstr();
  }
  astring PathStr::GetExtension() const {
    return ext_.GetWstr();
  }
  Void PathStr::SetDir(Wstr _dir) {
    dir_.Set(_dir);
  }
  Void PathStr::SetName(Wstr _name) {
    name_.Set(_name);
  }
  Void PathStr::SetExtension(Wstr _ext) {
    ext_.Set(_ext);
  }

  FileProp::FileProp() { }
  FileProp::FileProp(Wstr _name, Wstr _extension /*= L"afile"*/, Wstr _dir /*= GetCurrentDir().c_str()*/,
    FileOpenType _open_type /*= FileOpenType::kOpenExist*/, Dword _access_flags /*= kRead | kWrite*/,
    Dword _share_flags /*= kShareRead*/, Dword _attr_flags /*= kNormal*/, Int _handling_bytes_at_once /*= 512*/)
    : open_type_(_open_type), access_flagset_(_access_flags), share_flagset_(_share_flags),
    attr_flagset_(_attr_flags), handling_bytes_at_once_(_handling_bytes_at_once) 
  {
    path_.SetDir(_dir);
    path_.SetName(_name);
    path_.SetExtension(_extension);
  }
  FileProp::~FileProp() {
    _TRACE("FileProp destroy."); 
  }

}//!namespace atug2