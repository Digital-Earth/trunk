<?php
$user = $_SERVER['HTTP_USER'] or die("No user specified");
$maxSize = 20000000;
if($_FILES['file']['size'] > $maxSize) {
  echo "Log file size exceeds the limit of " . $maxSize;
  return;
}

$uploadDir = './logFiles/';
$uploadFile = $uploadDir . $user . date("YmdHis") . '.log';

if (move_uploaded_file($_FILES['file']['tmp_name'], $uploadFile)) {
  echo "Log file successfully uploaded.\n";
} else {
  echo "Possible file upload attack!\n";
}
?>
