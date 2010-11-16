
cd ../controller

for i in 0 1 2 3 4 5
do
  ./upk --define  package-$i service-$i ../demo/sleep10.sh
#  ./upk --set-actual-state package-$i service-$i start
  ./upk --set-desired-state package-$i service-$i start
  # sleep 1
done
