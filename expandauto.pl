#!/usr/bin/perl

use FindBin;

open($config, "<", "$FindBin::Bin/Makefile.config");
while ($line = <$config>) {
	if ($line =~ /^KLEE_BUILD_PATH=(.*)/) {
		$KLEE_BUILD_PATH = $1;
	}
}

$outdir = shift @ARGV;

sub get_single_cmd {
	my ($output_dir, $cmd) = @_;
	return "$KLEE_BUILD_PATH/bin/klee -libc=uclibc -output-source=false -output-istats=false -output-dir=$output_dir -instrument-simd $FindBin::Bin/$cmd";
}

my $printfh;

sub get_output_dir {
	my ($base, $id, $kind) = @_;
	return "$outdir/$base.auto.out/klee-out-$id-$kind";
}

sub print_single_cmd {
	my ($base, $id, $kind, $cmd, $fh) = @_;
	my $single_cmd = get_single_cmd(get_output_dir($base, $id, $kind), $cmd);
	print $fh "$single_cmd > $outdir/$base.auto.out/$id-$kind.out 2> $outdir/$base.auto.out/$id-$kind.err\n";
}

sub make_cmd_script {
	my ($base, $id, $kind, $cmd) = @_;
	my $single_cmd = get_single_cmd(get_output_dir($base, $id, $kind), $cmd);
	open(my $cmdscrfh, '>', "$outdir/$base.scripts/run-$id-$kind");

	print $cmdscrfh "#!/bin/sh\n\n";

	print $cmdscrfh "#PBS -l walltime=1:00:00\n";
	print $cmdscrfh "#PBS -l mem=1000mb\n";
	print $cmdscrfh "#PBS -k n\n\n";

	print $cmdscrfh "$single_cmd > $outdir/$base.auto.out/$id-$kind.log 2> $outdir/$base.auto.out/$id-$kind.err\n";

	close($cmdscrfh);
	chmod 0755, "$outdir/$base.scripts/run-$id-$kind";
}

sub build_cmd_for_dim {
	my ($autoscr, $base, $kind, $cmdpart, $action, $arg) = @_;

	open($auto, '-|', "./$autoscr") or die $?;
	while ($line = <$auto>) {
		chomp $line;
		my $id; my $cmd;
		($id, $cmd) = split / /, $line, 2;
		&$action($base, $id, $kind, "$cmd $cmdpart", $arg);
	}
}

for $autoscr (@ARGV) {
	$base = $autoscr;
	$base =~ s/\.auto//g;
	$base =~ s/^.*\///g;

	open($autoscrfh, "<", $autoscr);
	$autoscrcontent = join "", <$autoscrfh>;
	close($autoscrfh);

	if ($autoscrcontent =~ /MAX-DIM=([0-9]+)/) {
		$max_dim = $1;
	} else {
		$max_dim = undef;
	}

	if ($autoscrcontent =~ /MAX-DIM1=([0-9]+)/) {
		$max_dim1 = $1;
	} else {
		$max_dim1 = undef;
	}

	if ($autoscrcontent =~ /MAX-DIM2=([0-9]+)/) {
		$max_dim2 = $1;
	} else {
		$max_dim2 = undef;
	}

	my $batch = 1;
	if ($batch) {
		mkdir "$outdir/$base.scripts";
		mkdir "$outdir/$base.auto.out";
		$action = "make_cmd_script";
	} else {
		open($printfh, ">", "$outdir/$base.script");

		print $printfh "#!/bin/sh\n\n";

		print $printfh "rm -rf $outdir/$base.auto.out\n";
		print $printfh "mkdir $outdir/$base.auto.out\n\n";

		$action = "print_single_cmd";
	}

	if (defined $max_dim1) {
		for my $width (4..$max_dim1) {
			for my $height (1..$max_dim1) {
				for my $width2 (4..$max_dim2) {
					for my $height2 (1..$max_dim2) {
						build_cmd_for_dim($autoscr, $base, "${width}x${height}-${width2}x${height2}", "-w $width -h $height -W $width2 -H $height2", $action, $printfh);
					}
				}
			}
		}
	} elsif (defined $max_dim) {
		for my $width (4..$max_dim) {
			for my $height (1..$max_dim) {
				build_cmd_for_dim($autoscr, $base, "${width}x${height}", "-w $width -h $height", $action, $printfh);
			}
		}
	} else {
		build_cmd_for_dim($autoscr, $base, "n", "", $action, $printfh);
	}

	if (!$batch) {
		close($printfh);
		chmod 0755, "$outdir/$base.script";
	}
}
