# buddy perl test script
chdir "store";
system "./init.sh";

chdir "../buddy";
system "./buddytest";
