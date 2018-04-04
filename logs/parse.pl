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

my %instrs;
my %cycles;
my %access;
my %refill;

my @pids = keys %runtime;
foreach my $pid (@pids) {
    print "runtime for PID: $pid\n";
    my $total_instrs = [0, 0];
    my $total_cycles = [0, 0];
    my $total_access = [0, 0];
    my $total_refill = [0, 0];
    for ($cpu = 0; $cpu < $numcpus; $cpu++) {
        my $data_usr = $runtime{$pid}{$cpu}{0};
        my $data_sys = $runtime{$pid}{$cpu}{1};
        # print "@{$runtime{$pid}{$cpu}{0}}[0]\n";
        # print "@{$runtime{$pid}{$cpu}{1}}[0]\n";

        @$total_instrs[0] += @$data_usr[0];
        @$total_instrs[1] += @$data_sys[0];

        @$total_cycles[0] += @$data_usr[1];
        @$total_cycles[1] += @$data_sys[1];

        @$total_access[0] += @$data_usr[2];
        @$total_access[1] += @$data_sys[2];
        
        @$total_refill[0] += @$data_usr[3];
        @$total_refill[1] += @$data_sys[3];
    }

    $instrs{$pid} = $total_instrs;
    $cycles{$pid} = $total_cycles;
    $access{$pid} = $total_access;
    $refill{$pid} = $total_refill;

    # my $throughput  = [ @$total_instrs[0]/@$total_cycles[0],
    #                     @$total_instrs[1]/@$total_cycles[1] ];
    # my $refill_rate = [ @$total_access[0]/@$total_refill[0],
    #                     @$total_access[1]/@$total_refill[1] ];
    # print "[@$total_instrs], [@$total_cycles] = $throughput\n";
    # print "[@$total_access], [@$total_refill] = $refill_rate\n";

    # print "@$throughput[0]\n";
    # print "@$throughput[1]\n";
    # print "-----------------\n";
    # print "@$refill_rate[0]\n";
    # print "@$refill_rate[1]\n";
}

my $total_instrs = [0, 0];
my $total_cycles = [0, 0];
my $total_access = [0, 0];
my $total_refill = [0, 0];
for my $pid (@pids) {
    @$total_instrs[0] += @{$instrs{$pid}}[0];
    @$total_instrs[1] += @{$instrs{$pid}}[1];
    @$total_cycles[0] += @{$cycles{$pid}}[0];
    @$total_cycles[1] += @{$cycles{$pid}}[1];
    @$total_access[0] += @{$access{$pid}}[0];
    @$total_access[1] += @{$access{$pid}}[1];
    @$total_refill[0] += @{$refill{$pid}}[0];
    @$total_refill[1] += @{$refill{$pid}}[1];
}

print "totals:\n"
print "@$total_instrs[0]\n";
print "@$total_instrs[1]\n";
print "-----------------\n";
print "@$total_cycles[0]\n";
print "@$total_cycles[1]\n";
print "-----------------\n";
print "@$total_access[0]\n";
print "@$total_access[1]\n";
print "-----------------\n";
print "@$total_refill[0]\n";
print "@$total_refill[1]\n";
print "-----------------\n";

# $, = " ";
# print "@{$runtime{$pid}{$cpu}{$sys}}\n";