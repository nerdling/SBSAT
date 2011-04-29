# SBSAT

A state-based satisfiability solver.

Used primarily for solving instances of a generalization of the well-known Satisﬁability problem.

Homepage: <http://gauss.ececs.uc.edu/franco_files/sbsat.html>

## Quick start

 1. `./configure`
 2. `make`
 3. `make check`

## Internal Compatibility testing:

 - i386 Linux RH7.3 and EL WS3
 - i386 Windows Cygwin
 - Sparc Solaris 9
 - G4 Mac OS X 10.3.5

## To create a CD:

    mkisofs -o /SBSatCD1.8/sbsatcd18.iso -V SBSat_1_8 -r -J -hfs /work/CD

## test it before burning:

    mount -o loop,ro /SBSatCD1.8/sbsatcd18.iso /mnt
    umount /mnt
    /sbin/losetup -d /dev/loop0

