#!/bin/sh -e

echo 'Running autoreconf -if...'
(
	autoreconf -if
)
echo 'Running automake --add-missing'
(
	automake --add-missing
)
