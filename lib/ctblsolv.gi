#############################################################################
##
#W  ctblsolv.gi                 GAP library                Hans Ulrich Besche
#W                                                              Thomas Breuer
##
#H  @(#)$Id$
##
#Y  Copyright (C)  1997,  Lehrstuhl D fuer Mathematik,  RWTH Aachen,  Germany
##
##  This file contains character table methods for solvable groups.
##
Revision.ctblsolv_gi :=
    "@(#)$Id$";


#############################################################################
##
#M  CharacterDegreesOp( <G>, <p> )  . . . . . . . . . .  for an abelian group
##
InstallMethod( CharacterDegreesOp,
    "method for an abelian group, and an integer",
    true,
    [ IsGroup and IsAbelian, IsInt ], 0,
    function( G, p )
    G:= Size( G );
    if p <> 0 then
      while G mod p = 0 do
        G:= G / p;
      od;
    fi;
    return [ [ 1, G ] ];
    end );


#############################################################################
##
#F  AppendCollectedList( <list1>, <list2> )
##
AppendCollectedList := function( list1, list2 )
    local pair1, pair2, toadd;
    for pair2 in list2 do
      toadd:= true;
      for pair1 in list1 do
        if pair1[1] = pair2[1] then
          pair1[2]:= pair1[2] + pair2[2];
          toadd:= false;
          break;
        fi;
      od;
      if toadd then
        AddSet( list1, pair2 );
      fi;
    od;
end;


#############################################################################
##
#F  KernelUnderDualAction( <N>, <Npcgs>, <v> )  . . . . . . .  local function
##
##  <Npcgs> is a PCGS of an elementary abelian group <N>.
##  <v> is a vector in the dual space of <N>, w.r.t. <Npcgs>.
##  The kernel of <v> is returned.
##
KernelUnderDualAction := function( N, Npcgs, v )

    local gens, # generators list
          i, j;

    gens:= [];
    for i in Reversed( [ 1 .. Length( v ) ] ) do
      if IsZero( v[i] ) then
        Add( gens, Npcgs[i] );
      else
        # 'i' is the position of the last nonzero entry of 'v'.
        for j in Reversed( [ 1 .. i-1 ] ) do
          Add( gens, Npcgs[j]*Npcgs[i]^( Int(-v[j]/v[i]) ) );
        od;
        return SubgroupNC( N, Reversed( gens ) );
      fi;
    od;
end;


#############################################################################
##
#F  ProjectiveCharDeg( <G> ,<z> ,<q> )
##
##  is a collected list of the degrees of those faithful and absolutely
##  irreducible characters of the group <G> in characteristic <q> that
##  restrict homogeneously to the group generated by <z>, which must be
##  central in <G>.
##  Only those characters are counted that have value a multiple of
##  'E( Order(<z>) )' on <z>.
##
ProjectiveCharDeg := function( G, z, q )

    local oz,       # the order of 'z'
          N,        # normal subgroup of 'G'
          t,
          r,        # collected list of character degrees, result
          h,        # natural epimorphism
          k,
          c,
          ci,
          zn,
          i,
          p,        # prime divisor of the size of 'N'
          P,        # Sylow 'p' subgroup of 'N'
          O,
          L,
          Gpcgs,    # PCGS of 'G'
          Ppcgs,    # PCGS of 'P'
          Opcgs,    # PCGS of 'O'
          mats,
          orbs,
          orb,      # loop over 'orbs'
          stab;     # stabilizer of canonical representative of 'orb'

    oz:= Order( z );

    # For abelian groups, there are only linear characters.
    if IsAbelian( G ) then
      G:= Size( G );
      if q <> 0 then
        while G mod q = 0 do
          G:= G / q;
        od;
      fi;
      return [ [ 1, G/oz ] ];
    fi;

    # Now 'G' is not abelian.
    h:= NaturalHomomorphismByNormalSubgroup( G, SubgroupNC( G, [ z ] ) );
    N:= ElementaryAbelianSeries( Range( h ) );
    N:= N[ Length( N )-1 ];
    if not IsPrime( Size( N ) ) then
      N:= ChiefSeriesUnderAction( Range( h ), N );
      N:= N[ Length( N )-1 ];
    fi;

    # 'N' is a normal subgroup such that 'N/<z>' is a chief factor of 'G'
    # of order 'i' which is a power of 'p'.
    N:= PreImagesSet( h, N );
    i:= Size( N ) / oz;
    p:= Factors( i )[1];

    if not IsAbelian( N ) then

      h:= NaturalHomomorphismByNormalSubgroup( G, SubgroupNC( G, [ z ] ) );

      # 'c' is a list of complement classes of 'N' modulo 'z'
      c:= List( Complementclasses( Range( h ), ImagesSet( h, N ) ),
                x -> PreImagesSet( h, x ) );
      r:= Centralizer( G, N );
      for L in c do
        if IsSubset( L, r ) then

          # L is a complement to N in G modulo <z> which centralizes N
          r:= RootInt( Size(N) / oz );
          return List( ProjectiveCharDeg( L, z, q ),
                       x -> [ x[1]*r, x[2] ] );

        fi;
      od;
      Error( "this should not happen" );

    fi;

    # 'N' is abelian, 'P' is its Sylow 'p' subgroup.
    P:= SylowSubgroup( N, p );

    if p = q then

      # Factor out 'P' (lies in the kernel of the repr.)
      h:= NaturalHomomorphismByNormalSubgroup( G, P );
      return ProjectiveCharDeg( Range( h ), ImageElm( h, z ), q );

    elif i = Size( P ) then 

      # 'z' is a p'-element, 'P' is elementary abelian.
      # Find the characters of the factor group needed.
      h:= NaturalHomomorphismByNormalSubgroup( G, P );
      r:= ProjectiveCharDeg( Range( h ), ImageElm( h, z ), q );

      if p = i then

        # 'P' has order 'p'.
        zn:= First( GeneratorsOfGroup( P ), g -> not IsOne( g ) );
        t:=  Stabilizer( G, zn );
        i:= Size(G) / Size(t);
        AppendCollectedList( r,
            List( ProjectiveCharDeg( t, zn*z, q ),
                  x -> [ x[1]*i, x[2]*(p-1)/i ] ) );
        return r;

      else

        # 'P' has order strictly larger than 'p'.
        # 'mats' describes the contragredient operation of 'G' on 'P'.
        Gpcgs:= Pcgs( G );
        Ppcgs:= Pcgs( P );
        mats:= List( List( Gpcgs, Inverse ),
                   x -> TransposedMat( List( Ppcgs,
                   y -> ExponentsOfPcElement( Ppcgs, y^x ) )*Z(p)^0 ) );
        orbs:= ExternalOrbitsStabilizers( G,
                   Enumerator( FullRowModule( GF(p), Length( Ppcgs ) ) ),
                   Gpcgs, mats, OnRight );
        orbs:= Filtered( orbs,
              o -> not IsZero( CanonicalRepresentativeOfExternalSet( o ) ) );
                                       
        for orb in orbs do

          # 'k' is the kernel of the character.
          stab:= StabilizerOfExternalSet( orb );
          h:= NaturalHomomorphismByNormalSubgroup( stab,
                  KernelUnderDualAction( P, Ppcgs,
                      CanonicalRepresentativeOfExternalSet( orb ) ) );

          # 'zn' is an element of 'Range( h )'.
          # Note that the image of 'P' under 'h' has order 'p'.
          zn:= First( GeneratorsOfGroup( ImagesSet( h, P) ),
                      g -> not IsOne( g ) )
               * ImageElm( h, z );

          # 'c' is stabilizer of the character,
          # 'ci' is the number of orbits of characters with equal kernels
          if p = 2 then
            c  := Range( h );
            ci := 1;
          else
            c  := Stabilizer( Range( h ), zn );
            ci := Size( Range( h ) ) / Size( c );
          fi;
          k:= Size( G ) / Size( stab ) * ci;
          AppendCollectedList( r,
              List( ProjectiveCharDeg( c, zn, q ),
                    x -> [ x[1]*k, x[2]*(p-1)/ci ] ) );

        od;
        return r;

      fi;

    elif IsCyclic( P ) then

      # Choose a generator 'zn' of 'P'.
      zn := Pcgs( P )[1];
      t  := Stabilizer( G, zn, OnPoints );
      if G = t then
        # 'P' is a central subgroup of 'G'.
        return List( ProjectiveCharDeg( G, zn*z, q ),
                     x -> [ x[1], x[2]*p ] );
      else
        # 'P' is not central in 'G'.
        return List( ProjectiveCharDeg( t, zn*z, q ),
                     x -> [ x[1]*p, x[2] ] );
      fi;

    fi;

    # 'P' is the direct product of the Sylow 'p' subgroup of 'z'
    # and an elementary abelian 'p' subgroup.
    O:= Omega( P );
    Opcgs:= Pcgs( O );
    Gpcgs:= Pcgs( G );

    # 'zn' is a generator of the intersection of <z> and 'O'
    zn := z^(oz/p);
    r  := [];
    mats:= List( List( Gpcgs, Inverse ),
                 x -> TransposedMat( List( Opcgs,
                      y -> ExponentsOfPcElement( Opcgs, y^x ) ) * Z(p)^0 ) );
    orbs:= ExternalOrbitsStabilizers( G,
               Enumerator( GF(p)^Length( Opcgs ) ),
               Gpcgs, mats, OnRight );
    orbs:= Filtered( orbs,
              o -> not IsZero( CanonicalRepresentativeOfExternalSet( o ) ) );
                                     
    # In this case the stabilzers of the kernels are already the
    # stabilizers of the characters.
    for orb in orbs do
      k:= KernelUnderDualAction( O, Opcgs,
              CanonicalRepresentativeOfExternalSet( orb ) );
      if not zn in k then
        # The kernel avoids 'zn'.
        t:= StabilizerOfExternalSet( orb );
        h:= NaturalHomomorphismByNormalSubgroup( t, k );
        t:= Size(G) / Size(t);
        AppendCollectedList( r, List( ProjectiveCharDeg( Range( h ),
                                          ImageElm( h, z ), q ),
                                      x -> [ x[1]*t, x[2] ] ) );
      fi;
    od;
    return r;
end;


#############################################################################
##
#M  CharacterDegreesOp( <G>, <p> )  . . . . . . . . . .  for a solvable group
##
##  The used algorithm is described in
##
##      S. B. Conlon, J. Symbolic Computation (1990) 9, 551-570.
##
##  The main theoretic tool for the algorithm is Clifford theory.
##  One recursive step of the algorithm will be described.
##
##  Let $G$ be a solvable group, $z$ a central element in $G$,
##  and let $q$ be the characteristic of the algebraic closed field $F$.
##  Without loss of generality, we may assume that $G$ is nonabelian.
##  Consider a faithful linear character $\lambda$ of $\langle z \rangle$.
##  We calculate the character degrees $(G,z,q)$ of those absolutely
##  irreducible characters of $G$ whose restrictions to $\langle z \rangle$
##  are a multiple of $\lambda$.
##
##  We choose a normal subgroup $N$ of $G$ such that the factor
##  $N / \langle z \rangle$ is a chief factor in $G$, and consider
##  the following cases.
##
##  If $N$ is nonabelian then we calculate a subgroup $L$ of $G$ such that
##  $N \cap L = \langle z \rangle$, $L$ centralizes $N$, and $N L = G$.
##  One can show that the order of $N / \langle z \rangle$ is a square $r^2$,
##  and that the degrees $(G,z,q)$ are obtained from the degrees $(L,z,q)$
##  on multiplying each with $r$.
##
##  If $N$ is abelian then the order of $N / \langle z \rangle$ is a prime
##  power $p^i$.
##  Let $P$ denote the Sylow $p$ subgroup of $N$.
##  Following Clifford's theorem, we calculate orbit representatives and
##  inertia subgroups with respect to the action of $G$ on those irreducible
##  characters of $P$ that restrict to multiples of $\lambda_P$.
##  For that, we distinguish three cases.
##
##  (a) $z$ is a $p^{\prime}$ element.
##      Then we compute first the character degrees $(G/P,zP,q)$,
##      corresponding to the (orbit of the) trivial character.
##      The action on the nontrivial irreducible characters of $P$
##      is dual to the action on the nonzero vectors of the vector space
##      $P$.
##      For each representative, we compute the kernel $K$, and the degrees
##      $(S/K,zK,q)$, where $S$ denotes the inertia subgroup.
##
##  (b) $z$ is not a $p^{\prime}$ element, and $P$ cyclic (not prime order).
##      Let $y$ be a generator of $P$.
##      If $y$ is central in $G$ then we have to return $p$ copies of the
##      degrees $(G,zy,q)$.
##      Otherwise we compute the degrees $(C_G(y),zy,q)$, and multiply
##      each with $p$.
##
##  (c) $z$ is not a $p^{\prime}$ element, and $P$ is not cyclic.
##      We compute $O = \Omega(P)$.
##      As above, we consider the dual operation to that in $O$,
##      and for each orbit representative we check whether its restriction
##      to $O$ is a multiple of $\lambda_O$, and if yes compute the degrees
##      $(S/K,zK,q)$.
##
##  Note that only those cases of the algorithm 'ProjectiveCharDeg'
##  are needed that occur for trivial $z$.
##  Especialy N is elementary abelian.
##
InstallMethod( CharacterDegreesOp,
    "method for a solvable group and an integer (Conlon's algorithm)",
    true,
    [ IsGroup and IsSolvableGroup, IsInt ], 0,
    function( G, q )

    local r,      # list of degrees, result
          N,      # elementary abelian normal subgroup of 'G'
          p,      # prime divisor of the order of 'N'
          z,      # one generator of 'N'
          t,      # stabilizer of 'z' in 'G'
          i,      # index of 't' in 'G'
          Gpcgs,  # PCGS of 'G'
          Npcgs,  # PCGS of 'N'
          mats,   # matrices describing the action of 'Gpcgs' w.r.t. 'Npcgs'
          orbs,   # orbits of the action
          orb,    # loop over 'orbs'
          rep,    # canonical representative of 'orb'
          stab,   # stabilkizer of 'rep'
          h,      # nat. hom. by the kernel of a character
          c,
          ci,
          k;

    # If the group is abelian, we must give up because this method
    # needs a proper elementary abelian normal subgroup for its
    # reduction step.
    # (Note that we must not call 'TryNextMethod' because the method
    # for abelian groups has higher rank.)
    if IsAbelian( G ) then
      return CharacterDegrees( G, q );
    elif not ( q = 0 or IsPrimeInt( q ) ) then
      Error( "<q> mut be zero or a prime" );
    fi;

    # Choose a normal elementary abelian 'p'-subgroup 'N',
    # not necessarily minimal.
    N:= ElementaryAbelianSeries( G );
    N:= N[ Length( N ) - 1 ];
    r:= CharacterDegreesOp( G / N, q );
    p:= Factors( Size( N ) )[1];

    if p = q then

      # If 'N' is a 'q'-group we are done.
      return r;

    elif Size( N ) = p then

      # 'N' is of prime order.
      z:= Pcgs( N )[1];
      t:= Stabilizer( G, z, OnPoints );
      i:= Size( G ) / Size( t );
      AppendCollectedList( r, List( ProjectiveCharDeg( t, z, q ),
                                    x -> [ x[1]*i, x[2]*(p-1)/i ] ) );
      return r;

    else

      # 'N' is an elementary abelian 'p'-group of nonprime order.
      Gpcgs:= Pcgs( G );
      Npcgs:= Pcgs( N );
      mats:= List( Gpcgs, x -> TransposedMat( List( Npcgs,
                 y -> ExponentsOfPcElement( Npcgs, y^x ) ) * Z(p)^0 )^-1 );
      orbs:= ExternalOrbitsStabilizers( G,
                 Enumerator( GF( p )^Length( Npcgs ) ),
                 Gpcgs, mats, OnRight );
      orbs:= Filtered( orbs,
              o -> not IsZero( CanonicalRepresentativeOfExternalSet( o ) ) );
                                     
      for orb in orbs do

        stab:= StabilizerOfExternalSet( orb );
        rep:= CanonicalRepresentativeOfExternalSet( orb );
        h:= NaturalHomomorphismByNormalSubgroup( stab,
                KernelUnderDualAction( N, Npcgs, rep ) );
        # The kernel has index 'p' in 'stab'.
        z:= First( GeneratorsOfGroup( ImagesSet( h, N ) ),
                   g -> not IsOne( g ) );
        if p = 2 then
          c  := Range( h );
          ci := 1;
        else 
          c  := Stabilizer( Range( h ), z );
          ci := Size( Range( h ) ) / Size( c );
        fi;
        k:= Size( G ) / Size( stab ) * ci;
        AppendCollectedList( r, List( ProjectiveCharDeg( c, z, q ),
                                      x -> [ x[1]*k, x[2]*(p-1)/ci ] ) );

      od;

    fi;

    return r;
    end );


#############################################################################
##
#F  CoveringTriplesCharacters( <G>, <z> ) . . . . . . . . . . . . . . . local
##
##  <G> must be a supersolvable group, and <z> a central element in <G>.
##  'CoveringTriplesCharacters' returns a list of tripels $[ T, K, e ]$
##  such that every irreducible character $\chi$ of <G> with the property
##  that $\chi(<z>)$ is a multiple of 'E( Order(<z>) )' is induced from a
##  linear character of some $T$, with kernel $K$.
##  The element $e \in T$ is chosen such that $\langle e K \rangle = T/K$.
##
##  The algorithm is in principle the same as 'ProjectiveCharDeg',
##  but the recursion stops if $<G> = <z>$.
##  The structure and the names of the variables are the same.
##
CoveringTriplesCharacters := function( G, z )

    local oz,
          N,
          t,
          r,
          h,
          k,
          c,
          zn,
          i,
          p,
          P,
          O,
          Gpcgs,
          Ppcgs,
          Opcgs,
          mats,
          orbs,
          orb;

    # The trivial character will be dealt with separately.
    if IsTrivial( G ) then
      return [];
    fi;

    oz:= Order( z );
    if Size( G ) = oz then
      return [ [ G, TrivialSubgroup( G ), z ] ];
    fi;

    h:= NaturalHomomorphismByNormalSubgroup( G, SubgroupNC( G, [ z ] ) );
    N:= ElementaryAbelianSeries( Range( h ) );
    N:= N[ Length( N ) - 1 ];
    if not IsPrime( Size( N ) ) then
      N:= ChiefSeriesUnderAction( Range( h ), N );
      N:= N[ Length( N ) - 1 ];
    fi;
    N:= PreImagesSet( h, N );

    if not IsAbelian( N ) then
      Print( "#I misuse of 'CoveringTriplesCharacters'!\n" );
      return [];
    fi;

    i:= Size( N ) / oz;
    p:= Factors( i )[1];
    P:= SylowSubgroup( N, p );

    if i = Size( P ) then 

      # 'z' is a p'-element, 'P' is elementary abelian.
      # Find the characters of the factor group needed.
      h:= NaturalHomomorphismByNormalSubgroup( G, P );
      r:= List( CoveringTriplesCharacters( Range( h ), ImageElm( h, z ) ),
                x -> [ PreImagesSet( h, x[1] ),
                       PreImagesSet( h, x[2] ),
                       PreImagesRepresentative( h, x[3] ) ] );

      if p = i then

        # 'P' has order 'p'.
        zn:= First( GeneratorsOfGroup( P ), g -> not IsOne( g ) );
        return Concatenation( r,
                   CoveringTriplesCharacters( Stabilizer( G, zn ), zn*z ) );

      else

        Gpcgs:= Pcgs( G );
        Ppcgs:= Pcgs( P );
        mats:= List( List( Gpcgs, Inverse ),
                   x -> TransposedMat( List( Ppcgs,
                   y -> ExponentsOfPcElement( Ppcgs, y^x ) )*Z(p)^0 ) );
        orbs:= ExternalOrbitsStabilizers( G,
                   Enumerator( FullRowModule( GF(p), Length( Ppcgs ) ) ),
                   Gpcgs, mats, OnRight );
        orbs:= Filtered( orbs,
              o -> not IsZero( CanonicalRepresentativeOfExternalSet( o ) ) );
                                       
        for orb in orbs do
          h:= NaturalHomomorphismByNormalSubgroup(
                  StabilizerOfExternalSet( orb ),
                  KernelUnderDualAction( P, Ppcgs,
                      CanonicalRepresentativeOfExternalSet( orb ) ) );
          zn:= First( GeneratorsOfGroup( ImagesSet( h, P ) ),
                      g -> not IsOne( g ) )
               * ImageElm( h, z );
  
          if p = 2 then
            c:= Range( h );
          else
            c:= Stabilizer( Range( h ), zn );
          fi;
          Append( r, List( CoveringTriplesCharacters( c, zn ),
                           x -> [ PreImagesSet( h, x[1] ),
                                  PreImagesSet( h, x[2] ),
                                  PreImagesRepresentative( h, x[3] ) ] ) );
        od;
        return r;

      fi;

    elif IsCyclic( P ) then

      zn:= Pcgs( P )[1];
      return CoveringTriplesCharacters( Stabilizer( G, zn ), zn*z );

    fi;

    O:= Omega( P );
    Opcgs:= Pcgs( O );
    Gpcgs:= Pcgs( G );

    zn := z^(oz/p);
    r  := [];
    mats:= List( List( Gpcgs, Inverse ),
                 x -> TransposedMat( List( Opcgs,
                      y -> ExponentsOfPcElement( Opcgs, y^x ) )*Z(p)^0 ) );
    orbs:= ExternalOrbitsStabilizers( G,
               Enumerator( FullRowModule( GF(p), Length( Opcgs ) ) ),
               Gpcgs, mats, OnRight );
    orbs:= Filtered( orbs,
              o -> not IsZero( CanonicalRepresentativeOfExternalSet( o ) ) );
                                     
    for orb in orbs do
      k:= KernelUnderDualAction( O, Opcgs,
              CanonicalRepresentativeOfExternalSet( orb ) );
      if not zn in k then
        t:= SubgroupNC( G, StabilizerOfExternalSet( orb ) );
        h:= NaturalHomomorphismByNormalSubgroup( t, k );
        Append( r,
            List( CoveringTriplesCharacters( Range( h ), ImageElm( h, z ) ),
                  x -> [ PreImagesSet( h, x[1] ),
                         PreImagesSet( h, x[2] ),
                         PreImagesRepresentative( h, x[3] ) ] ) );
      fi;
    od;
    return r;
end;


#############################################################################
##
#F  IrrConlon( <G> )
##
##  This algorithm is a generalization of the algorithm to compute the
##  absolutely irreducible degrees of a solvable group to the computation
##  of the absolutely irreducible characters of a supersolvable group,
##  using an idea like in 
##
##      S. B. Conlon, J. Symbolic Computation (1990) 9, 535-550.
##
##  The function 'CoveringTriplesCharacters' is used to compute a list of
##  triples describing linear representations of subgroups of <G>.
##  These linear representations are induced to <G> and then evaluated on
##  representatives of the conjugacy classes. 
##
##  For every irreducible character the monomiality information is stored as
##  value of the attribute 'TestMonomial'.
##
IrrConlon := function( G )

    local ccl,        # conjugacy classes of 'G'
          Gpcgs,      # PCGS of 'G'
          irr,        # matrix of character values
          irredinfo,  # monomiality info
          evl,        # encode class representatives as words in 'Gpcgs'
          i,
          t,
          chi,
          j,
          mat,
          k,
          triple,
          hom,
          zi,
          oz,
          ee,
          zp,
          co,          # cosets
          coreps,      # representatives of 'co'
          dim,
          rep,         # matrix representation
          bco,
          p,
          r,
          mulmoma,     # local function: multiply monomial matrices
          i1,          # loop variable in 'mulmoma'
          re,          # result of 'mulmoma'
          ct;          # character table of 'G'

    # Compute the product of the monomial matrices 'a' and 'b';
    # The diagonal elements are powers of a fixed 'oz'-th root of unity.
    mulmoma:= function( a, b )
      re:= rec( perm:= [], diag:= [] );
      for i1 in [ 1 .. Length( a.perm ) ] do
        re.perm[i1]:= b.perm[ a.perm[i1] ];
        re.diag[ b.perm[i1] ]:= ( b.diag[ b.perm[i1] ] + a.diag[i1] ) mod oz;
      od;
      return re;
    end;

    ccl:= ConjugacyClasses( G );
    Gpcgs:= Pcgs( G );
    irr:= [];
    irredinfo:= [ rec( inducedFrom:= rec( subgroup:= G, kernel:= G ) ) ];

    # 'evl' is a list describing representatives of the nontrivial
    # conjugacy classes.
    # the entry for the element $g.1^2*g.2^0*g.3^1$ is $[ 1, 1, 3 ]$.
    evl:= [];
    for i in [ 2 .. Length( ccl ) ] do
      k:= ExponentsOfPcElement( Gpcgs, Representative( ccl[i] ) );
      t:= [];
      for j in [ 1 .. Length( k ) ] do
        if 0 < k[j] then
          Append( t, [ 1 .. k[j] ]*0 + j );
        fi;
      od;
      Add( evl, t );
    od;

    for triple in CoveringTriplesCharacters( G, One( G ) ) do

      hom:= NaturalHomomorphismByNormalSubgroup( triple[1], triple[2] );
      zi:= ImagesRepresentative( hom, triple[3] );
      oz:= Order( zi );
      ee:= E( oz );
      zp:= List( [ 1 .. oz ], x -> zi^x );
      co:= RightCosets( G, triple[1] );
      coreps:= List(  co, Representative );
      dim:= Length( co );

      # 'rep' describes a matrix representation on a module with basis
      # a transversal of the stabilizer in 'G'.
      # (The monomial matrices are the same as in 'RepresentationsPGroup'.)
      rep:= [];
      for i in Gpcgs do
        mat:= rec( perm:= [], diag:= [] );
        for j in [ 1 .. dim ] do
          bco:= co[j]*i;
          p:= Position( co, bco, 0 );
          Add( mat.perm, p );
          mat.diag[p]:= Position( zp,
              ImageElm( hom, coreps[j]*i*Inverse( coreps[p] ) ), 0 );
        od;
        Add( rep, mat );
      od;

      # Compute the representing matrices for class representatives,
      # and their traces.
      chi:= [ dim ];
      for j in evl do
        mat:= Iterated( rep{ j }, mulmoma );
        t:= 0;
        for k in [ 1 .. dim ] do
          if mat.perm[k] = k then 
            t:= t + ee^mat.diag[k];
          fi;
        od;
        Add( chi, t );
      od;

      # Test if 'chi' is known and add 'chi' and its Galois-conjugates
      # to the list.
      # Also compute the monomiality information.
      if not chi in irr then
        chi:= GaloisMat( [ chi ] ).mat;
        Append( irr, chi );
        for j in chi do
          Add( irredinfo, rec( subgroup:= triple[1], kernel:= triple[2] ) );
        od;
      fi;

    od;

    # Construct the characters from their values lists,
    # and set the monomiality info.
    ct:= CharacterTable( G );
    irr:= Concatenation( [ TrivialCharacter( G ) ],
                         List( irr, chi -> CharacterByValues( ct, chi ) ) );
    for i in [ 1 .. Length( irr ) ] do
      SetTestMonomial( irr[i], irredinfo[i] );
    od;

    # Return the characters.
    return irr;
end;


#############################################################################
##
#M  Irr( <G> )  . . . . . . .  for a supersolvable group (Conlon's algorithm)
##
InstallMethod( Irr,
    "method for a supersolvable group (Conlon's algorithm)",
    true,
    [ IsGroup and IsSupersolvableGroup ], 0,
    IrrConlon );


#############################################################################
##
#E  ctblsolv.gi . . . . . . . . . . . . . . . . . . . . . . . . . . ends here



