#!/usr/local/bin/perl

my $out;
my $file = "stdout";
my $i;
my $total = 0;
for ($i=0; $i<5;$i++){
        `make clean`;
        if(-e $file) {
                `rm -rf $file`;
        }
        $out = `(/usr/bin/time make) 2>> stdout`;
        open (TXT, "<", "$file") or die ("Cannot open file\n");
        while (<TXT>){
                if(/(.*)user/){
                        print "$1\n";
                        $total += $1;
                }
        }
}
$total = $total/5;
print "$total\n";