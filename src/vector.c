/****************************************************************************
**
*A  vector.c                    GAP source                   Martin Schoenert
**
*H  @(#)$Id$
**
*Y  Copyright (C)  1996,  Lehrstuhl D fuer Mathematik,  RWTH Aachen,  Germany
**
**  This file contains the functions  that mainly  operate  on vectors  whose
**  elements are integers, rationals, or elements from cyclotomic fields.  As
**  vectors are special lists many things are done in the list package.
**
**  A *vector* is a list that has no holes,  and whose elements all come from
**  a common field.  For the full definition of vectors see chapter "Vectors"
**  in  the {\GAP} manual.   Read also about "More   about Vectors" about the
**  vector flag and the compact representation of vectors over finite fields.
*/
char *          Revision_vector_c =
   "@(#)$Id$";

#include        "system.h"              /* system dependent functions      */

#include        "gasman.h"              /* NewBag, ResizeBag, CHANGED_BAG  */
#include        "objects.h"             /* Obj, TYPE_OBJ, SIZE_OBJ, ...    */
#include        "scanner.h"             /* Pr                              */

#include        "ariths.h"              /* generic operations package      */
#include        "lists.h"               /* generic list package            */

#include        "bool.h"                /* True, False                     */

#include        "integer.h"             /* TypDigit                        */

#include        "listoper.h"            /* ProdListScl                     */

#include        "plist.h"               /* GET_LEN_PLIST, GET_ELM_PLIST,...*/

#define INCLUDE_DECLARATION_PART
#include        "vector.h"              /* declaration part of the package */
#undef  INCLUDE_DECLARATION_PART

#include        "range.h"               /* GET_LEN_RANGE, GET_LOW_RANGE,...*/

#include        "gap.h"                 /* Error                           */


/****************************************************************************
**
*F  IsXTypeEmpty(<list>)  . test if a list is an empty list (almost a vector)
*/
Int             IsXTypeEmpty (
    Obj                 list )
{
    return (LEN_LIST( list ) == 0);
}


/****************************************************************************
**
*F  IsXTypePlistCyc(<list>) . . . . . . . . . . .  test if a list is a vector
**
**  'IsXTypePlistCyc'  returns 1  if   the list <list>  is   a vector  and  0
**  otherwise.    As a  sideeffect    the type of  the   list  is  changed to
**  'T_VECTOR'.
**
**  'IsXTypePlistCyc' is the function in 'IsXTypeListFuncs' for vectors.
*/
#define IS_IMM_PLIST(list)  ((TYPE_OBJ(list) - T_PLIST) % 2)

Int             IsXTypePlistCyc (
    Obj                 list )
{
    Int                 isVector;       /* result                          */
    UInt                len;            /* length of the list              */
    Obj                 elm;            /* one element of the list         */
    UInt                i;              /* loop variable                   */

    /* if we already know that the list is a vector, very good             */
    if      ( T_PLIST_CYC    <= TYPE_OBJ(list)
           && TYPE_OBJ(list) <= T_PLIST_CYC_SSORT +IMMUTABLE ) {
        isVector = 1;
    }

    /* if it is a nonempty plain list, check the entries                   */
    else if ( (TYPE_OBJ(list) == T_PLIST
            || TYPE_OBJ(list) == T_PLIST +IMMUTABLE
            || TYPE_OBJ(list) == T_PLIST_DENSE
            || TYPE_OBJ(list) == T_PLIST_DENSE +IMMUTABLE
            || (T_PLIST_HOM <= TYPE_OBJ(list)
             && TYPE_OBJ(list) <= T_PLIST_HOM_SSORT +IMMUTABLE))
           && LEN_PLIST(list) != 0
           && ELM_PLIST(list,1) != 0
           && TYPE_OBJ( ELM_PLIST(list,1) ) <= T_CYC ) {
        len = LEN_PLIST(list);
        for ( i = 2; i <= len; i++ ) {
            elm = ELM_PLIST( list, i );
            if ( elm == 0
              || ! (TYPE_OBJ(elm) <= T_CYC) )
                break;
        }
        isVector = (len < i) ? 1 : 0;
        if ( isVector )  RetypeBag( list, T_PLIST_CYC + IS_IMM_PLIST(list) );
    }

    /* a range is a vector, but we have to convert it                      */
    /*N 1993/01/30 martin finds it nasty that vector knows about ranges    */
    else if ( TYPE_OBJ(list) == T_RANGE_NSORT ) {
        PLAIN_LIST( list );
        RetypeBag( list, T_PLIST_CYC_NSORT + IS_IMM_PLIST(list) );
        isVector = 1;
    }
    else if ( TYPE_OBJ(list) == T_RANGE_SSORT ) {
        PLAIN_LIST( list );
        RetypeBag( list, T_PLIST_CYC_SSORT + IS_IMM_PLIST(list) );
        isVector = 1;
    }

    /* otherwise the list is certainly not a vector                        */
    else {
        isVector = 0;
    }

    /* return the result                                                   */
    return isVector;
}


/****************************************************************************
**
*F  IsXTypeMatCyc(<list>) . . . . . . . . . . . .  test if a list is a matrix
**
**  'IsXTypeMatCyc' returns 1 if the list <list> is a matrix and 0 otherwise.
**  As a sideeffect the type of the rows is changed to 'T_VECTOR'.
**
**  'IsXTypeMatCyc' is the function in 'IsXTypeListFuncs' for matrices.
*/
Int             IsXTypeMatCyc (
    Obj                 list )
{
    Int                 isMatrix;       /* result                          */
    UInt                cols;           /* length of the rows              */
    UInt                len;            /* length of the list              */
    Obj                 elm;            /* one element of the list         */
    UInt                i;              /* loop variable                   */

    /* if it is a nonempty plain list, check the entries                   */
    if ( (TYPE_OBJ(list) == T_PLIST
       || TYPE_OBJ(list) == T_PLIST +IMMUTABLE
       || TYPE_OBJ(list) == T_PLIST_DENSE
       || TYPE_OBJ(list) == T_PLIST_DENSE +IMMUTABLE
       || (T_PLIST_HOM <= TYPE_OBJ(list)
        && TYPE_OBJ(list) <= T_PLIST_HOM_SSORT +IMMUTABLE))
      && LEN_PLIST( list ) != 0
      && ELM_PLIST( list, 1 ) != 0
      && IsXTypePlistCyc( ELM_PLIST( list, 1 ) ) ) {
        len = LEN_PLIST( list );
        elm = ELM_PLIST( list, 1 );
        cols = LEN_PLIST( elm );
        for ( i = 2; i <= len; i++ ) {
            elm = ELM_PLIST( list, i );
            if ( elm == 0
              || ! IsXTypePlistCyc( elm )
              || LEN_PLIST( elm ) != cols )
                break;
        }
        isMatrix = (len < i) ? 1 : 0;
    }

    /* otherwise the list is certainly not a matrix                        */
    else {
        isMatrix = 0;
    }

    /* return the result                                                   */
    return isMatrix;
}


/****************************************************************************
**
*F  SumIntVector(<elmL>,<vecR>) . . . . . . .  sum of an integer and a vector
**
**  'SumIntVector' returns the   sum of the   integer <elmL>  and  the vector
**  <vecR>.  The sum is a  list, where each element is  the sum of <elmL> and
**  the corresponding element of <vecR>.
**
**  'SumIntVector' is an improved version  of  'SumSclList', which  does  not
**  call 'SUM' if the operands are immediate integers.
*/
Obj             SumIntVector (
    Obj                 elmL,
    Obj                 vecR )
{
    Obj                 vecS;           /* handle of the sum               */
    Obj *               ptrS;           /* pointer into the sum            */
    Obj                 elmS;           /* one element of sum list         */
    Obj *               ptrR;           /* pointer into the right operand  */
    Obj                 elmR;           /* one element of right operand    */
    UInt                len;            /* length                          */
    UInt                i;              /* loop variable                   */

    /* make the result list                                                */
    len = LEN_PLIST( vecR );
    vecS = NEW_PLIST( T_PLIST_CYC, len );
    SET_LEN_PLIST( vecS, len );

    /* loop over the elements and add                                      */
    ptrR = ADDR_OBJ( vecR );
    ptrS = ADDR_OBJ( vecS );
    for ( i = 1; i <= len; i++ ) {
        elmR = ptrR[i];
        if ( ! ARE_INTOBJS(elmL,elmR) || ! SUM_INTOBJS(elmS,elmL,elmR) ) {
            CHANGED_BAG( vecS );
            elmS = SUM( elmL, elmR );
            ptrR = ADDR_OBJ( vecR );
            ptrS = ADDR_OBJ( vecS );
        }
        ptrS[i] = elmS;
    }

    /* return the result                                                   */
    CHANGED_BAG( vecS );
    return vecS;
}


/****************************************************************************
**
*F  SumVectorInt(<vecL>,<elmR>) . . . . . . .  sum of a vector and an integer
**
**  'SumVectorInt' returns  the sum of   the  vector <vecL> and  the  integer
**  <elmR>.  The sum is a  list, where each element  is the sum of <elmR> and
**  the corresponding element of <vecL>.
**
**  'SumVectorInt' is an improved version  of  'SumListScl', which  does  not
**  call 'SUM' if the operands are immediate integers.
*/
Obj             SumVectorInt (
    Obj                 vecL,
    Obj                 elmR )
{
    Obj                 vecS;           /* handle of the sum               */
    Obj *               ptrS;           /* pointer into the sum            */
    Obj                 elmS;           /* one element of sum list         */
    Obj *               ptrL;           /* pointer into the left operand   */
    Obj                 elmL;           /* one element of left operand     */
    UInt                len;            /* length                          */
    UInt                i;              /* loop variable                   */

    /* make the result list                                                */
    len = LEN_PLIST( vecL );
    vecS = NEW_PLIST( T_PLIST_CYC, len );
    SET_LEN_PLIST( vecS, len );

    /* loop over the elements and add                                      */
    ptrL = ADDR_OBJ( vecL );
    ptrS = ADDR_OBJ( vecS );
    for ( i = 1; i <= len; i++ ) {
        elmL = ptrL[i];
        if ( ! ARE_INTOBJS(elmL,elmR) || ! SUM_INTOBJS(elmS,elmL,elmR) ) {
            CHANGED_BAG( vecS );
            elmS = SUM( elmL, elmR );
            ptrL = ADDR_OBJ( vecL );
            ptrS = ADDR_OBJ( vecS );
        }
        ptrS[i] = elmS;
    }

    /* return the result                                                   */
    CHANGED_BAG( vecS );
    return vecS;
}


/****************************************************************************
**
*F  SumVectorVector(<vecL>,<vecR>)  . . . . . . . . . . .  sum of two vectors
**
**  'SumVectorVector' returns the sum  of the two  vectors <vecL> and <vecR>.
**  The sum is a new list, where each element is the sum of the corresponding
**  elements of <vecL> and <vecR>.
**
**  'SumVectorVector' is an improved version of 'SumListList', which does not
**  call 'SUM' if the operands are immediate integers.
*/
Obj             SumVectorVector (
    Obj                 vecL,
    Obj                 vecR )
{
    Obj                 vecS;           /* handle of the sum               */
    Obj *               ptrS;           /* pointer into the sum            */
    Obj                 elmS;           /* one element of sum list         */
    Obj *               ptrL;           /* pointer into the left operand   */
    Obj                 elmL;           /* one element of left operand     */
    Obj *               ptrR;           /* pointer into the right operand  */
    Obj                 elmR;           /* one element of right operand    */
    UInt                len;            /* length                          */
    UInt                i;              /* loop variable                   */

    /* make the result list                                                */
    len = LEN_PLIST( vecL );
    if ( len != LEN_PLIST( vecR ) ) {
        vecR = ErrorReturnObj(
             "Vector +: <right> must have the same length as <left> (%d)",
             (Int)len, 0L,
             "you can return a new vector for <right>" );
        return SUM( vecL, vecR );
    }
    vecS = NEW_PLIST( T_PLIST_CYC, len );
    SET_LEN_PLIST( vecS, len );

    /* loop over the elements and add                                      */
    ptrL = ADDR_OBJ( vecL );
    ptrR = ADDR_OBJ( vecR );
    ptrS = ADDR_OBJ( vecS );
    for ( i = 1; i <= len; i++ ) {
        elmL = ptrL[i];
        elmR = ptrR[i];
        if ( ! ARE_INTOBJS(elmL,elmR) || ! SUM_INTOBJS(elmS,elmL,elmR) ) {
            CHANGED_BAG( vecS );
            elmS = SUM( elmL, elmR );
            ptrL = ADDR_OBJ( vecL );
            ptrR = ADDR_OBJ( vecR );
            ptrS = ADDR_OBJ( vecS );
        }
        ptrS[i] = elmS;
    }

    /* return the result                                                   */
    CHANGED_BAG( vecS );
    return vecS;
}


/****************************************************************************
**
*F  DiffIntVector(<elmL>,<vecR>)  . . . difference of an integer and a vector
**
**  'DiffIntVector' returns  the difference  of  the integer  <elmL> and  the
**  vector <vecR>.   The difference  is  a list,  where  each element is  the
**  difference of <elmL> and the corresponding element of <vecR>.
**
**  'DiffIntVector'  is an improved  version of 'DiffSclList', which does not
**  call 'DIFF' if the operands are immediate integers.
*/
Obj             DiffIntVector (
    Obj                 elmL,
    Obj                 vecR )
{
    Obj                 vecD;           /* handle of the difference        */
    Obj *               ptrD;           /* pointer into the difference     */
    Obj                 elmD;           /* one element of difference list  */
    Obj *               ptrR;           /* pointer into the right operand  */
    Obj                 elmR;           /* one element of right operand    */
    UInt                len;            /* length                          */
    UInt                i;              /* loop variable                   */

    /* make the result list                                                */
    len = LEN_PLIST( vecR );
    vecD = NEW_PLIST( T_PLIST_CYC, len );
    SET_LEN_PLIST( vecD, len );

    /* loop over the elements and subtract                                 */
    ptrR = ADDR_OBJ( vecR );
    ptrD = ADDR_OBJ( vecD );
    for ( i = 1; i <= len; i++ ) {
        elmR = ptrR[i];
        if ( ! ARE_INTOBJS(elmL,elmR) || ! DIFF_INTOBJS(elmD,elmL,elmR) ) {
            CHANGED_BAG( vecD );
            elmD = DIFF( elmL, elmR );
            ptrR = ADDR_OBJ( vecR );
            ptrD = ADDR_OBJ( vecD );
        }
        ptrD[i] = elmD;
    }

    /* return the result                                                   */
    CHANGED_BAG( vecD );
    return vecD;
}


/****************************************************************************
**
*F  DiffVectorInt(<vecL>,<elmR>)  . . . difference of a vector and an integer
**
**  'DiffVectorInt' returns   the  difference of the  vector  <vecL>  and the
**  integer <elmR>.  The difference   is a list,   where each element  is the
**  difference of <elmR> and the corresponding element of <vecL>.
**
**  'DiffVectorInt' is an improved  version of 'DiffListScl', which  does not
**  call 'DIFF' if the operands are immediate integers.
*/
Obj             DiffVectorInt (
    Obj                 vecL,
    Obj                 elmR )
{
    Obj                 vecD;           /* handle of the difference        */
    Obj *               ptrD;           /* pointer into the difference     */
    Obj                 elmD;           /* one element of difference list  */
    Obj *               ptrL;           /* pointer into the left operand   */
    Obj                 elmL;           /* one element of left operand     */
    UInt                len;            /* length                          */
    UInt                i;              /* loop variable                   */

    /* make the result list                                                */
    len = LEN_PLIST( vecL );
    vecD = NEW_PLIST( T_PLIST_CYC, len );
    SET_LEN_PLIST( vecD, len );

    /* loop over the elements and subtract                                 */
    ptrL = ADDR_OBJ( vecL );
    ptrD = ADDR_OBJ( vecD );
    for ( i = 1; i <= len; i++ ) {
        elmL = ptrL[i];
        if ( ! ARE_INTOBJS(elmL,elmR) || ! DIFF_INTOBJS(elmD,elmL,elmR) ) {
            CHANGED_BAG( vecD );
            elmD = DIFF( elmL, elmR );
            ptrL = ADDR_OBJ( vecL );
            ptrD = ADDR_OBJ( vecD );
        }
        ptrD[i] = elmD;
    }

    /* return the result                                                   */
    CHANGED_BAG( vecD );
    return vecD;
}


/****************************************************************************
**
*F  DiffVectorVector(<vecL>,<vecR>) . . . . . . . . difference of two vectors
**
**  'DiffVectorVector'  returns the difference of the  two vectors <vecL> and
**  <vecR>.   The  difference is   a new   list, where  each  element  is the
**  difference of the corresponding elements of <vecL> and <vecR>.
**
**  'DiffVectorVector' is an improved  version of  'DiffListList', which does
**  not call 'DIFF' if the operands are immediate integers.
*/
Obj             DiffVectorVector (
    Obj                 vecL,
    Obj                 vecR )
{
    Obj                 vecD;           /* handle of the difference        */
    Obj *               ptrD;           /* pointer into the difference     */
    Obj                 elmD;           /* one element of difference list  */
    Obj *               ptrL;           /* pointer into the left operand   */
    Obj                 elmL;           /* one element of left operand     */
    Obj *               ptrR;           /* pointer into the right operand  */
    Obj                 elmR;           /* one element of right operand    */
    UInt                len;            /* length                          */
    UInt                i;              /* loop variable                   */

    /* make the result list                                                */
    len = LEN_PLIST( vecL );
    if ( len != LEN_PLIST( vecR ) ) {
        vecR = ErrorReturnObj(
             "Vector -: <right> must have the same length as <left> (%d)",
             (Int)len, 0L,
             "you can return a new vector for <right>" );
        return DIFF( vecL, vecR );
    }
    vecD = NEW_PLIST( T_PLIST_CYC, len );
    SET_LEN_PLIST( vecD, len );

    /* loop over the elements and subtract                                 */
    ptrL = ADDR_OBJ( vecL );
    ptrR = ADDR_OBJ( vecR );
    ptrD = ADDR_OBJ( vecD );
    for ( i = 1; i <= len; i++ ) {
        elmL = ptrL[i];
        elmR = ptrR[i];
        if ( ! ARE_INTOBJS(elmL,elmR) || ! DIFF_INTOBJS(elmD,elmL,elmR) ) {
            CHANGED_BAG( vecD );
            elmD = DIFF( elmL, elmR );
            ptrL = ADDR_OBJ( vecL );
            ptrR = ADDR_OBJ( vecR );
            ptrD = ADDR_OBJ( vecD );
        }
        ptrD[i] = elmD;
    }

    /* return the result                                                   */
    CHANGED_BAG( vecD );
    return vecD;
}


/****************************************************************************
**
*F  ProdIntVector(<elmL>,<vecR>)  . . . .  product of an integer and a vector
**
**  'ProdIntVector' returns the product of the integer  <elmL> and the vector
**  <vecR>.  The product is  the list, where  each element is the product  of
**  <elmL> and the corresponding entry of <vecR>.
**
**  'ProdIntVector'  is an  improved version of 'ProdSclList', which does not
**  call 'PROD' if the operands are immediate integers.
*/
Obj             ProdIntVector (
    Obj                 elmL,
    Obj                 vecR )
{
    Obj                 vecP;           /* handle of the product           */
    Obj *               ptrP;           /* pointer into the product        */
    Obj                 elmP;           /* one element of product list     */
    Obj *               ptrR;           /* pointer into the right operand  */
    Obj                 elmR;           /* one element of right operand    */
    UInt                len;            /* length                          */
    UInt                i;              /* loop variable                   */

    /* make the result list                                                */
    len = LEN_PLIST( vecR );
    vecP = NEW_PLIST( T_PLIST_CYC, len );
    SET_LEN_PLIST( vecP, len );

    /* loop over the entries and multiply                                  */
    ptrR = ADDR_OBJ( vecR );
    ptrP = ADDR_OBJ( vecP );
    for ( i = 1; i <= len; i++ ) {
        elmR = ptrR[i];
        if ( ! ARE_INTOBJS(elmL,elmR) || ! PROD_INTOBJS(elmP,elmL,elmR) ) {
            CHANGED_BAG( vecP );
            elmP = PROD( elmL, elmR );
            ptrR = ADDR_OBJ( vecR );
            ptrP = ADDR_OBJ( vecP );
        }
        ptrP[i] = elmP;
    }

    /* return the result                                                   */
    CHANGED_BAG( vecP );
    return vecP;
}


/****************************************************************************
**
*F  ProdVectorInt(<vecL>,<elmR>)  . . . .  product of a scalar and an integer
**
**  'ProdVectorInt' returns the product of the integer  <elmR> and the vector
**  <vecL>.  The  product is the  list, where each element  is the product of
**  <elmR> and the corresponding element of <vecL>.
**
**  'ProdVectorInt'  is an  improved version of 'ProdSclList', which does not
**  call 'PROD' if the operands are immediate integers.
*/
Obj             ProdVectorInt (
    Obj                 vecL,
    Obj                 elmR )
{
    Obj                 vecP;           /* handle of the product           */
    Obj *               ptrP;           /* pointer into the product        */
    Obj                 elmP;           /* one element of product list     */
    Obj *               ptrL;           /* pointer into the left operand   */
    Obj                 elmL;           /* one element of left operand     */
    UInt                len;            /* length                          */
    UInt                i;              /* loop variable                   */

    /* make the result list                                                */
    len = LEN_PLIST( vecL );
    vecP = NEW_PLIST( T_PLIST_CYC, len );
    SET_LEN_PLIST( vecP, len );

    /* loop over the entries and multiply                                  */
    ptrL = ADDR_OBJ( vecL );
    ptrP = ADDR_OBJ( vecP );
    for ( i = 1; i <= len; i++ ) {
        elmL = ptrL[i];
        if ( ! ARE_INTOBJS(elmL,elmR) || ! PROD_INTOBJS(elmP,elmL,elmR) ) {
            CHANGED_BAG( vecP );
            elmP = PROD( elmL, elmR );
            ptrL = ADDR_OBJ( vecL );
            ptrP = ADDR_OBJ( vecP );
        }
        ptrP[i] = elmP;
    }

    /* return the result                                                   */
    CHANGED_BAG( vecP );
    return vecP;
}


/****************************************************************************
**
*F  ProdVectorVector(<vecL>,<vecR>) . . . . . . . . .  product of two vectors
**
**  'ProdVectorVector'  returns the product  of   the two vectors <vecL>  and
**  <vecR>.  The  product  is the  sum of the   products of the corresponding
**  elements of the two lists.
**
**  'ProdVectorVector' is an improved version  of 'ProdListList',  which does
**  not call 'PROD' if the operands are immediate integers.
*/
Obj             ProdVectorVector (
    Obj                 vecL,
    Obj                 vecR )
{
    Obj                 elmP;           /* product, result                 */
    Obj                 elmS;           /* partial sum of result           */
    Obj                 elmT;           /* one summand of result           */
    Obj *               ptrL;           /* pointer into the left operand   */
    Obj                 elmL;           /* one element of left operand     */
    Obj *               ptrR;           /* pointer into the right operand  */
    Obj                 elmR;           /* one element of right operand    */
    UInt                len;            /* length                          */
    UInt                i;              /* loop variable                   */

    /* check that the lengths agree                                        */
    len = LEN_PLIST( vecL );
    if ( len != LEN_PLIST( vecR ) ) {
        vecR = ErrorReturnObj(
             "Vector *: <right> must have the same length as <left> (%d)",
             (Int)len, 0L,
             "you can return a new vector for <right>" );
        return PROD( vecL, vecR );
    }

    /* loop over the entries and multiply                                  */
    ptrL = ADDR_OBJ( vecL );
    ptrR = ADDR_OBJ( vecR );
    elmL = ptrL[1];
    elmR = ptrR[1];
    if ( ! ARE_INTOBJS(elmL,elmR) || ! PROD_INTOBJS(elmT,elmL,elmR) ) {
        elmT = PROD( elmL, elmR );
        ptrL = ADDR_OBJ( vecL );
        ptrR = ADDR_OBJ( vecR );
    }
    elmP = elmT;
    for ( i = 2; i <= len; i++ ) {
        elmL = ptrL[i];
        elmR = ptrR[i];
        if ( ! ARE_INTOBJS(elmL,elmR) || ! PROD_INTOBJS(elmT,elmL,elmR) ) {
            elmT = PROD( elmL, elmR );
            ptrL = ADDR_OBJ( vecL );
            ptrR = ADDR_OBJ( vecR );
        }
        if ( ! ARE_INTOBJS(elmP,elmT) || ! SUM_INTOBJS(elmS,elmP,elmT) ) {
            elmS = SUM( elmP, elmT );
            ptrL = ADDR_OBJ( vecL );
            ptrR = ADDR_OBJ( vecR );
        }
        elmP = elmS;
    }

    /* return the result                                                   */
    return elmP;
}


/****************************************************************************
**
*F  ProdVectorMatrix(<vecL>,<vecR>) . . . . .  product of a vector and a matrix
**
**  'ProdVectorMatrix' returns the product of the vector <vecL> and the matrix
**  <vecR>.  The product is the sum of the  rows  of <vecR>, each multiplied by
**  the corresponding entry of <vecL>.
**
**  'ProdVectorMatrix'  is an improved version of 'ProdListList',  which does
**  not  call 'PROD' and  also accummulates  the sum into  one  fixed  vector
**  instead of allocating a new for each product and sum.
*/
Obj             ProdVectorMatrix (
    Obj                 vecL,
    Obj                 matR )
{
    Obj                 vecP;           /* handle of the product           */
    Obj *               ptrP;           /* pointer into the product        */
    Obj                 elmP;           /* one summand of product          */
    Obj                 elmS;           /* temporary for sum               */
    Obj                 elmT;           /* another temporary               */
    Obj                 elmL;           /* one element of left operand     */
    Obj                 vecR;           /* one vector of right operand     */
    Obj *               ptrR;           /* pointer into the right vector   */
    Obj                 elmR;           /* one element from right vector   */
    UInt                len;            /* length                          */
    UInt                col;            /* length of the rows              */
    UInt                i, k;           /* loop variables                  */

    /* check the lengths                                                   */
    len = LEN_PLIST( vecL );
    col = LEN_PLIST( ELM_PLIST( matR, 1 ) );
    if ( len != LEN_PLIST( matR ) ) {
        vecR = ErrorReturnObj(
             "Vector *: <right> must have the same length as <left> (%d)",
             (Int)len, 0L,
             "you can return a new matrix for <right>" );
        return PROD( vecL, vecR );
    }

    /* make the result list by multiplying the first entries               */
    elmL = ELM_PLIST( vecL, 1 );
    vecR = ELM_PLIST( matR, 1 );
    vecP = PROD( elmL, vecR );

    /* loop over the other entries and multiply                            */
    for ( i = 2; i <= len; i++ ) {
        elmL = ELM_PLIST( vecL, i );
        vecR = ELM_PLIST( matR, i );
        ptrR = ADDR_OBJ( vecR );
        ptrP = ADDR_OBJ( vecP  );
        if ( elmL == INTOBJ_INT( 1L ) ) {
            for ( k = 1; k <= col; k++ ) {
                elmT = ptrR[k];
                elmP = ptrP[k];
                if ( ! ARE_INTOBJS(elmP,elmT)
                  || ! SUM_INTOBJS(elmS,elmP,elmT) ) {
                    CHANGED_BAG( vecP );
                    elmS = SUM( elmP, elmT );
                    ptrR = ADDR_OBJ( vecR );
                    ptrP = ADDR_OBJ( vecP );
                }
                ptrP[k] = elmS;
            }
        }
        else if ( elmL == INTOBJ_INT( -1L ) ) {
            for ( k = 1; k <= col; k++ ) {
                elmT = ptrR[k];
                elmP = ptrP[k];
                if ( ! ARE_INTOBJS(elmP,elmT)
                  || ! DIFF_INTOBJS(elmS,elmP,elmT) ) {
                    CHANGED_BAG( vecP );
                    elmS = DIFF( elmP, elmT );
                    ptrR = ADDR_OBJ( vecR );
                    ptrP = ADDR_OBJ( vecP );
                }
                ptrP[k] = elmS;
            }
        }
        else if ( elmL != INTOBJ_INT( 0L ) ) {
            for ( k = 1; k <= col; k++ ) {
                elmR = ptrR[k];
                if ( ! ARE_INTOBJS(elmL,elmR)
                  || ! PROD_INTOBJS(elmT,elmL,elmR) ) {
                    CHANGED_BAG( vecP );
                    elmT = PROD( elmL, elmR );
                    ptrR = ADDR_OBJ( vecR );
                    ptrP = ADDR_OBJ( vecP );
                }
                elmP = ptrP[k];
                if ( ! ARE_INTOBJS(elmP,elmT)
                  || ! SUM_INTOBJS(elmS,elmP,elmT) ) {
                    CHANGED_BAG( vecP );
                    elmS = SUM( elmP, elmT );
                    ptrR = ADDR_OBJ( vecR );
                    ptrP = ADDR_OBJ( vecP );
                }
                ptrP[k] = elmS;
            }
        }
    }

    /* return the result                                                   */
    CHANGED_BAG( vecP );
    return vecP;
}


/****************************************************************************
**
*F  InitVector()  . . . . . . . . . . . . . . . . . initialize vector package
**
**  'InitVector' initializes the vector package.
*/
void            InitVector ( void )
{
    Int                 t1;
    Int                 t2;

    IsXTypeListFuncs[ T_PLIST_EMPTY  ] = IsXTypeEmpty;
    IsXTypeListFuncs[ T_PLIST_CYC    ] = IsXTypePlistCyc;
    IsXTypeListFuncs[ T_MAT_CYC      ] = IsXTypeMatCyc;

    /* install the arithmetic operation methods                            */
    for ( t1 = T_PLIST_CYC; t1 <= T_PLIST_CYC_SSORT; t1++ ) {
        for ( t2 = T_PLIST_CYC; t2 <= T_PLIST_CYC_SSORT; t2++ ) {
            SumFuncs [ T_INT     ][ t2        ] = SumIntVector;
            SumFuncs [ t1        ][ T_INT     ] = SumVectorInt;
            SumFuncs [ t1        ][ t2        ] = SumVectorVector;
            DiffFuncs[ T_INT     ][ t2        ] = DiffIntVector;
            DiffFuncs[ t1        ][ T_INT     ] = DiffVectorInt;
            DiffFuncs[ t1        ][ t2        ] = DiffVectorVector;
            ProdFuncs[ T_INT     ][ t2        ] = ProdIntVector;
            ProdFuncs[ t1        ][ T_INT     ] = ProdVectorInt;
            ProdFuncs[ t1        ][ t2        ] = ProdVectorVector;
            ProdFuncs[ t1        ][ T_MAT_CYC ] = ProdVectorMatrix;
        }
    }

}



