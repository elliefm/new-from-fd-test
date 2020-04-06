#!/usr/bin/env perl

use warnings;
use strict;

use IO::Handle;

my $pipe = IO::Handle->new_from_fd(3, "w");

if (defined $pipe) {
    print STDERR "child: pipe found, fd=" . fileno($pipe). "\n";

    print $pipe "ok";
    print STDERR "child: wrote 'ok' to pipe\n";

    print STDERR "child: about to close pipe, expect parent to see EOF\n";
    $pipe->close();
    undef $pipe;
    print STDERR "child: closed pipe, has parent seen EOF yet?\n";

    print STDERR "child: sleeping a while to see what parent does...\n";
    sleep 10;
    print STDERR "child: exiting\n"
}
else {
    print STDERR "child: no pipe found\n";
}
