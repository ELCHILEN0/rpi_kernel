#!/usr/bin/perl

use strict;
use warnings;

my $logfile = $ARGV[0];
my $numcpus = $ARGV[1];

open(my $in, "<", $logfile) or die "Can't open $logfile: $!";
my @lines = <$in>;

my %runtime;
my $pid;
my $sys;
my $cpu;
foreach(@lines) {
    if (/\w+\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)/) {
        if ($cpu == $numcpus) {
            $cpu = 0;
            $sys = 1;
        }
        $runtime{$pid}{$cpu}{$sys} = [$1, $2, $3, $4, $5, $6];
        $cpu += 1;

        # print;
    } elsif (/(\d+)\s+\[core \d\] exiting/) {
        # print "runtime for PID: $1\n";
        $pid = $1;
        $sys = 0;   
        $cpu = 0;     
    }
}

print "\n";

my @pids = keys %runtime;
foreach my $pid (@pids) {
    print "runtime for PID: $pid\n";
    my $total_cycles = [0, 0];
    for ($cpu = 0; $cpu < $numcpus; $cpu++) {
        my $data_usr = @{$runtime{$pid}{$cpu}{0}};
        my $data_sys = @{$runtime{$pid}{$cpu}{1}};
        print "@{$runtime{$pid}{$cpu}{0}}[0]\n";
        print "@{$runtime{$pid}{$cpu}{1}}[0]\n";

        # @$total_cycles[0] += @$data_usr[0];
        print $data_usr;
    }


}

# $, = " ";
# print "@{$runtime{$pid}{$cpu}{$sys}}\n";