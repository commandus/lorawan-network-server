<?php
$bands = file_get_contents("php://input");
$filename = 'gw-stat.txt';
file_put_contents($filename, $bands, FILE_APPEND);
echo "Write successful";
?>