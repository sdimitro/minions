         d   c       ,����������=Kȅm���!W��c            u#!/bin/sh

for i
do
	psu | awk '$NF ~ /^('$i')$/ {printf("/bin/kill -9 %d # %s\n", $2, $0);}'
done
     d     3   2     >    ����4��Y$�R�0�5�ۨig��0�            u#!/bin/sh

exec /usr/local/plan9/bin/kill -9 "$@"
     �     )   (     �   �����4��YO��z���~O�2���            u#!/bin/sh

exec $PLAN9/bin/kill -9 "$@"
