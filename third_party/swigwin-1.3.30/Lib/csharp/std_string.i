/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * std_string.i
 *
 * Typemaps for std::string and const std::string&
 * These are mapped to a C# String and are passed around by value.
 *
 * To use non-const std::string references use the following %apply.  Note 
 * that they are passed by value.
 * %apply const std::string & {std::string &};
 * ----------------------------------------------------------------------------- */

%{
#include <string>
%}

namespace std {

%naturalvar string;

class string;

// string
%typemap(ctype) string "UnmanagedString *" // c type
%typemap(imtype) string "IntPtr" // cs intermediary type
%typemap(cstype) string "string" // cs type

%typemap(csdirectorin) string "$modulePINVOKE.UnmanagedString.Consume($iminput)"
%typemap(csdirectorout) string "$modulePINVOKE.UnmanagedString.Create($cscall)"

%typemap(in) string %{
  // Generated from typemap(in) string
  ConsumeUnmanagedString($input, $1);
%}

%typemap(out) string %{
  // Generated from typemap(out) string
  $result = CreateUnmanagedString($1);
%}

%typemap(directorout) string %{
  // Generated from typemap(directorout) string
  ConsumeUnmanagedString($input, $result);
%}

%typemap(directorin) string %{
  // Generated from typemap(directorin) string
  $input = CreateUnmanagedString($1);
%}

%typemap(csin) string "$modulePINVOKE.UnmanagedString.Create($csinput)"

%typemap(csout, excode=SWIGEXCODE) string {
  // Generated from %typemap(csout) string
  string ret = $modulePINVOKE.UnmanagedString.Consume($imcall);
  $excode
  return ret;
}

%typemap(csvarin) string %{
  // Generated from %typemap(csvarin) string
  set {
    throw new NotImplementedException();
  }
%}

%typemap(csvarout, excode=SWIGEXCODE2) string %{
  // Generated from %typemap(csvarout) string
  get {
    string ret = $modulePINVOKE.UnmanagedString.Consume($imcall);
    $excode
    return ret;
  }
%}

// const string &
%typemap(ctype) const string & "UnmanagedString *" // ctype
%typemap(imtype) const string & "IntPtr" // cs intermediary type
%typemap(cstype) const string & "string" // cs type

%typemap(csdirectorin) const string & "$modulePINVOKE.UnmanagedString.Consume($iminput)"
%typemap(csdirectorout) const string & "$modulePINVOKE.UnmanagedString.Create($cscall)"

%typemap(in) const string & %{
  // Generated from typemap(in) const string &
  std::string str_$1;
  ConsumeUnmanagedString($input, str_$1);
  $1 = &str_$1;
%}

%typemap(out) const string & %{
  // Generated from typemap(out) const string &
  $result = CreateUnmanagedString(*$1);
%}

%typemap(directorout) const string & %{
  // Generated from typemap(directorout) const string &
  ConsumeUnmanagedString($input, $result);
%}

%typemap(directorin) const string & %{
  // Generated from typemap(directorin) const string &
  $input = CreateUnmanagedString($1);
%}

%typemap(csin) const string & "$modulePINVOKE.UnmanagedString.Create($csinput)"

%typemap(csout, excode=SWIGEXCODE) const string & {
  // Generated from typemap(csout) const string &
  string ret = $modulePINVOKE.UnmanagedString.Consume($imcall);
  $excode
  return ret;
}

%typemap(csvarin) const string & %{
  // Generated from %typemap(csvarin) const string &
  set {
    throw new NotImplementedException();
  }
%}

%typemap(csvarout, excode=SWIGEXCODE2) const string & %{
  // Generated from %typemap(csvarout) const string &
  get {
    string ret = $modulePINVOKE.UnmanagedString.Consume($imcall);
    $excode
    return ret;
  }
%}

}
