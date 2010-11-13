cd ../demo
ln -f ../buddy/buddy .
cd ../store
rm -f store.sqlite
./schema-setup.sh

cd ../controller
./upk --init

for i in 0 1 2 3 4 5
do
  ./upk --define  package-$i service-$i ../demo/sleep10.sh
  # ./upk --set-desired-state package-$i service-$i start
  # ./upk --set-desired-state package-$i service-$i stop
  # sleep 1
done
