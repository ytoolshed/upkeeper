
for i in 0 1 2 3 4 5
do
  ./upk --define  package-$i service-$i sleep 360$i
  ./upk --set-pid package-$i service-$i 12$i
  ./upk --set-actual-state package-$i service-$i start
  sleep 1
done
