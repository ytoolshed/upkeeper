
cd ../store
rm -f store.sqlite
./schema-setup.sh

cd ../controller
./upk --init
