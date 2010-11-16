
cd ../controller

for i in 0 1 2 3 4 5
do
#  ./upk --define  package-$i service-$i ""
#  ./upk --set-pid package-$i service-$i 0
#  ./upk --set-actual-state package-$i service-$i stop
   ./upk --set-desired-state package-$i service-$i stop
  # sleep 1
done

./upk --all-down
