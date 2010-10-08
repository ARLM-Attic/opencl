lower=[...
-11/2
 -6
 -9/2
-31/2
-29/2
-17/2
  1
 -1/2
 -2
-13/2
 -5
-21/2];

upper=[...
 7/2
 2
 5/2
-9/2
-3/2
 1/2
 9
17/2
 4
13/2
 3
 5/2];

delta = -(upper+lower)./2;

object = struct('type','flat',...
                'constraints',[constraint('leq le',[ 0  0  0  5 -4  delta( 1)],upper( 1)+delta( 1))
                               constraint('leq le',[ 0  0  5  0 -3  delta( 2)],upper( 2)+delta( 2))
                               constraint('leq le',[ 0  0  4 -3  0  delta( 3)],upper( 3)+delta( 3))
                               constraint('leq le',[ 0  4  0 -6  1  delta( 4)],upper( 4)+delta( 4))
                               constraint('leq le',[ 0  4 -8  0  1  delta( 5)],upper( 5)+delta( 5))
                               constraint('leq le',[ 0  3 -5  1  0  delta( 6)],upper( 6)+delta( 6))
                               constraint('leq le',[ 4  0  0  3 -1  delta( 7)],upper( 7)+delta( 7))
                               constraint('leq le',[ 4  0  4  0 -1  delta( 8)],upper( 8)+delta( 8))
                               constraint('leq le',[ 3  0  2 -1  0  delta( 9)],upper( 9)+delta( 9))
                               constraint('leq le',[ 8  4  0  0 -1  delta(10)],upper(10)+delta(10))
                               constraint('leq le',[ 5  2  0 -1  0  delta(11)],upper(11)+delta(11))
                               constraint('leq le',[ 8  2 -3  0  0  delta(12)],upper(12)+delta(12))]);