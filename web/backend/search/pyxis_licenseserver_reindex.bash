#!/bin/bash

OLD_VERSION=2
NEW_VERSION=3
HOST=http://localhost:9200
OLD_INDEX=pyxis_licenseserver_v$OLD_VERSION
NEW_INDEX=pyxis_licenseserver_v$NEW_VERSION
WRITE_ALIAS=pyxis_licenseserver
READ_ALIAS=pyxis_licenseserver_read
MAPPING=mapping/Resources_v${NEW_VERSION}.json
RUBY_DIR=/home/Pyxis/scripts/ruby

if [ ! -f $MAPPING ]; then
    echo "Mapping $MAPPING not found!  exiting..."
fi

# Make new index with desired mapping
curl -XPUT "$HOST/$NEW_INDEX" -d "`cat $MAPPING`"

# Shift write alias
curl -XPOST "$HOST/_aliases" -d "
{
    \"actions\" : [
        { \"remove\" : { \"index\" : \"$OLD_INDEX\", \"alias\" : \"$WRITE_ALIAS\" } },
        { \"add\" : { \"index\" : \"$NEW_INDEX\", \"alias\" : \"$WRITE_ALIAS\" } }
    ]
}"

# Reindex old documents
WD=`pwd`
cd $RUBY_DIR
echo -en "\n" | ruby es-reindex.rb $HOST/$OLD_INDEX $HOST/$NEW_INDEX
cd $WD

# Shift read alias
curl -XPOST '$HOST/_aliases' -d "
{
    \"actions\" : [
        { \"remove\" : { \"index\" : \"$OLD_INDEX\", \"alias\" : \"$READ_ALIAS\" } },
        { \"add\" : { \"index\" : \"$NEW_INDEX\", \"alias\" : \"$READ_ALIAS\" } }
    ]
}"

# Delete old index
curl -XDELETE "$HOST/$OLD_INDEX"

