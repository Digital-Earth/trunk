<?php

function validateImage($file, $maxSize) {
  if($_FILES[$file]['size'] > $maxSize) {
    http_response_code(403);
    exit("403 Image size exceeds the limit of " . $maxSize . "B.");
  }
  
  $imageType = exif_imagetype($_FILES[$file]['tmp_name']);
  if($imageType != IMAGETYPE_JPEG && $imageType != IMAGETYPE_PNG && $imageType != IMAGETYPE_GIF) {
    http_response_code(400);
    die("400 Unsupported image type. (JPEG, PNG, and GIF are supported)");
  }
  
  $size = getimagesize($_FILES[$file]['tmp_name']);
  if($size[0] == 0 || $size[1] == 0) {
    http_response_code(415);
    exit("415 Invalid image file.");
  }
}

function saveAndThumbnailImage($file, $uploadDir, $fileName, $minThumbnailWidth, $minThumbnailHeight) {
  $destination = $fileName;
  $destination = $uploadDir . $destination;
  if (move_uploaded_file($_FILES[$file]['tmp_name'], $destination)) {
    // delete other files with the same base name (in case extension changed)
    $sameBaseFiles = glob($uploadDir . pathinfo($fileName, PATHINFO_FILENAME) . ".*");
    if (($key = array_search($destination, $sameBaseFiles)) !== false) {
      unset($sameBaseFiles[$key]);
    }
    array_map('unlink', $sameBaseFiles);
    generateThumbnail($destination, $minThumbnailWidth, $minThumbnailHeight);
    http_response_code(200);
    echo "200";
  } else {
    http_response_code(403);
    echo "403 Possible upload attack!\n";
  }
}

function generateThumbnail($source, $minDestinationWidth, $minDestinationHeight) {
  list($sourceWidth, $sourceHeight) = getimagesize($source);
  $heightScale = $minDestinationHeight / (float) $sourceHeight;
  $widthScale = $minDestinationWidth / (float) $sourceWidth;
  if($heightScale > $widthScale) {
    $destinationHeight = $minDestinationHeight;
    $destinationWidth = $heightScale * $sourceWidth;
  }
  else {
    $destinationHeight = $widthScale * $sourceHeight;
    $destinationWidth = $minDestinationWidth;
  }
  $sepPos = strrpos($source, '/');
  $baseName = basename($source);
  $dotPos = strrpos($baseName, '.');
  $destination = substr($source, 0, $sepPos) . "/thumbnails/" . substr($baseName, 0, $dotPos) . ".jpg";

  $image = NULL;
  $imageType = exif_imagetype($source);
  if($imageType == IMAGETYPE_JPEG) {
    $image = imagecreatefromjpeg($source);
  }
  elseif($imageType == IMAGETYPE_PNG) {
    $image = imagecreatefrompng($source);
  }
  elseif($imageType == IMAGETYPE_GIF) {
    $image = imagecreatefromgif($source);
  }
  else {
    http_response_code(400);
    die("400 Unsupported image type. (JPEG, PNG, and GIF are supported)");
  }
  $tn = imagecreatetruecolor($destinationWidth, $destinationHeight);
  imagecopyresampled($tn, $image, 0, 0, 0, 0, $destinationWidth, $destinationHeight, $sourceWidth, $sourceHeight);

  imagejpeg($tn, $destination, 100);
}

function processImage($uploadDir, $maxImageFileSize, $minThumbnailWidth, $minThumbnailHeight) {
  reset($_FILES);
  $fileName = key($_FILES);

  validateImage($fileName, $maxImageFileSize);

  // . is converted to _ in the $_FILES object
  $normalizedFileName = $fileName;
  $dot = strrpos($fileName, '_');
  if($dot !== false) {
    $normalizedFileName[$dot] = '.';
  }
  saveAndThumbnailImage($fileName, $uploadDir, $normalizedFileName, $minThumbnailWidth, $minThumbnailHeight);
}

?>
