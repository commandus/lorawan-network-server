<?php
$values = file_get_contents("php://input");
$filename = 'gw-stat.txt';
file_put_contents($filename, $values, FILE_APPEND);
echo "Write successful";
?>