<?php
include '/home/pyxis/webapps/htdocs/data/catalogue/imageTools.php';

thumbnailGenerator("/home/pyxis/webapps/htdocs/images/pipelines/", 220, 0);
thumbnailGenerator("/home/pyxis/webapps/htdocs/images/avatars/", 96, 96);
thumbnailGenerator("/home/pyxis/webapps/htdocs/images/galleries/", 1320, 168);
thumbnailGenerator("/home/pyxis/webapps/htdocs/images/groups/", 300, 300);

function thumbnailGenerator($imagePath, $minDestinationWidth, $minDestinationHeight) {
  $dir = opendir($imagePath);

  while (false !== ($fname = readdir( $dir ))) {
    $imageType = exif_imagetype($imagePath . $fname);
    if($imageType == IMAGETYPE_JPEG || $imageType == IMAGETYPE_PNG || $imageType == IMAGETYPE_GIF) {
      generateThumbnail($imagePath . $fname, $minDestinationWidth, $minDestinationHeight);
    }
  }
}

?>
