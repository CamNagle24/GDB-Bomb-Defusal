# Read the following instructions carefully
# You will provide your solution to this part of the project by
# editing the collection of functions in this source file.
#
# Some rules from Project 2 are still in effect for your assembly code here:
#  1. No global variables are allowed
#  2. You may not define or call any additional functions in this file
#  3. You may not use any floating-point assembly instructions
# You may assume that your machine:
#  1. Uses two's complement, 32-bit representations of integers.

# bitMatch - Create mask indicating which bits in x match those in y
#   Example: bitMatch(0x7, 0xE) = 0x6
#   Rating: 1
.global bitMatch
bitMatch:
    movl %edi, %edx     # move x into edx
    andl %esi, %edx     # and y and x (one)
    notl %edi           # not x
    notl %esi           # not y
    andl %esi, %edi     # and not y and not x (two)
    orl %edi, %edx      # or one and two
    movl %edx, %eax     # move result into the return register
    ret

# evenBits - return word with all even-numbered bits set to 1
#   where bits are numbered from 0 (least significant) to 31 (most significant)
#   Rating: 1
.global evenBits
evenBits:
    movl $1431655765, %eax  # move the number with all even bits set to one into the return register
    ret

# allOddBits - return 1 if all odd-numbered bits in word set to 1
#   where bits are numbered from 0 (least significant) to 31 (most significant)
#   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
#   Rating: 2
.global allOddBits
allOddBits:
    movl $2863311530, %esi      # move the mask into %esi
    andl %esi, %edi             # and the mask and x and store in %edi
    xorl %edi, %esi             # or the anded result from above value the mask and store into esi
    cmpl $0, %esi               # compare to zero
    je ZERO                     # if it is not 0 (a number) then set 0 to eax and return
    movl $0, %eax               # else move 0 into return register and return
    ret
ZERO:                           # if it is 0 then set 1 to eax and return
    movl $1, %eax
    ret

# floatAbsVal - Return bit-level equivalent of absolute value of f for
#   floating point argument f.
#   Both the argument and result are passed as unsigned ints, but
#   they are to be interpreted as the bit-level representations of
#   single-precision floating point values.
#   When argument is NaN, return argument.
#   Rating: 2
.global floatAbsVal
floatAbsVal:
    movl %edi, %esi         # move uf into %esi
    movl %edi, %edx         # move uf into %edx
    sarl $23, %esi          # >> shift uf by 23 and put it in %esi
    andl $255, %esi         # and that shifted value with 255 and put it in %esi
    cmpl $255, %esi         # compare that with 255
    je EQUAL                # jump if equal to 255
    andl $0x7FFFFFFF, %edi      # if not equal to 255 then and uf with 0x7FFFFFFF
    movl %edi, %eax             # move that value to eax and return
    ret
    
EQUAL:
    andl $0x7FFFFF, %edi     # and 0x7FFFFF with uf and put it in edi
    cmpl $0, %edi              # compare 0 with that value
    jne NOT_EQUAL               # if not equal to 0 then jump
    andl $0x7FFFFFFF, %edx      # if equal to 0 then and uf with 0x7FFFFFFF
    movl %edx, %eax             # move that value to eax and return
    ret                         
    
NOT_EQUAL:
    movl %edx, %eax           # if (uf & 0x7FFFFF) != 0 move uf which is in edx to eax and return
    ret


# implication - return x -> y in propositional logic - 0 for false, 1 for true
#   Example: implication(1,1) = 1
#            implication(1,0) = 0
#   Rating: 2
.global implication
implication:
    cmpl $0, %edi               # compare 0 to x
    je ZERO_                    # if it is 0 then jump
    movl $0, %edi               # if not 0 set %edi to 0
    orl %edi, %esi              # or that with y (%esi)
    movl %esi, %eax             # move that value to %eax and return
    ret
ZERO_:                      # if the number is 0
    movl $1, %edi                  # move 1 to %edi
    orl %edi, %esi                 # or that with y (%esi)
    movl %esi, %eax                # move that value to %eax and return
    ret

# isNegative - return 1 if x < 0, return 0 otherwise
#   Example: isNegative(-1) = 1.
#   Rating: 2
.global isNegative
isNegative:
    sarl $31, %edi          # right shift x by 31
    andl $1, %edi           # and it with 1
    movl %edi, %eax         # move that into the return register
    ret

# sign - return 1 if positive, 0 if zero, and -1 if negative
#  Examples: sign(130) = 1
#            sign(-23) = -1
#  Rating: 2
.global sign
sign:
    movl %edi, %esi     # move x into %esi
    sarl $31, %edi      # right shift x by 31 and store in %edi
    cmpl $0, %esi       # compare 0 to x
    je IS_ZERO          # jump if x is equal to 0
    cmpl $0, %edi       # compare 0 with the sign_bit
    je IS_POSITIVE      # jump if sign_bit is positive
    movl $-1, %eax      # sign is negative
    ret
IS_POSITIVE:
    movl $1, %eax       # sign is positive
    ret
IS_ZERO:
    movl $0, %eax       # number is 0
    ret


# isGreater - if x > y  then return 1, else return 0
#   Example: isGreater(4,5) = 0, isGreater(5,4) = 1
#   Rating: 3
.global isGreater
isGreater:
    cmpl $0x7fffffff, %edi      # compare the max pos with x
    je X_MAX_POS                # jump if it is
    cmpl $0x80000000, %esi      # compare the min neg with y
    je Y_MIN_NEG                # jump if it is
    cmpl %esi, %edi             # compare x with y
    je NUMS_EQUAL               # jump if they are equal
    jg X_GREATER                # jump if x is greater
    movl $0, %eax               # move 0 to eax because y is greater and return
    ret
X_GREATER:
    movl $1, %eax               # x is greater so move 1 to eax and return
    ret
NUMS_EQUAL:
    movl $0, %eax               # if they are equal move 0 to eax and return
    ret
X_MAX_POS:
    movl $1, %eax               # move 1 to the return register
    cmpl $0x80000000, %esi      # compare y to the least negative value
    je Y_MIN_NEG                # jump if so
    cmpl $0x7fffffff, %esi      # compare max y to the max value
    je NUMS_EQUAL               # jump to equal if they are both max positive
    ret                         # return 1 if not
Y_MIN_NEG:
    movl $1, %eax               # move 1 to the return adress
    cmpl $0x80000000, %edi      # compare min value to x
    je NUMS_EQUAL               # jump if it is
    ret                         # return 1 if it is not

#  logicalShift - shift x to the right by n, using a logical shift
#    Can assume that 0 <= n <= 31
#    Examples: logicalShift(0x87654321,4) = 0x08765432
#    Rating: 3
.global logicalShift
logicalShift:
    movl %edi, %eax     # move x into %eax
    movl %esi, %ecx     # move n into %ecx
    shrl %cl, %eax      # right shift x by n bits
    sarl $31, %edx      # right shift 1 by 31 and put into %edx
    shrl %cl, %edx      # right shift 1 to the right by n bits
    notl %edx           # ~the bits of that
    andl %edx, %eax     # and the mask and the shifted value
    ret      

# rotateRight - Rotate x to the right by n
#   Can assume that 0 <= n <= 31
#   Examples: rotateRight(0x87654321,4) = 0x18765432
#   Rating: 3
.global rotateRight
rotateRight:
    movl $0, %ecx           # i = 0
    movl %esi, %edx         # move n into edx
    movl %edi, %eax         # move x into eax  
    LOOP1:
        cmpl %ecx, %esi     # if i == n
        je DONE1            # jump if the loop is done
        shrl $1, %eax       # right shift x by 1
        incl %ecx           # inc i
        jmp LOOP1           # start the loop again
    DONE1:
        movl $32, %edx      # move 32 into edx
        subl %esi, %edx     # subtract n from 32
        movl $0, %ecx       # move 0 into i
        jmp ROTATE          # do the rotation
    ROTATE:
        cmpl %ecx, %edx     # if i == n jump to end
        je END              
        shll $1, %edi       # left shift x by 1
        incl %ecx           # increase i by 1
        jmp ROTATE          # start the loop again
    END:
        orl %edi, %eax      # or subtracted n from 32 with x right shifted
        ret                 

# floatScale4 - Return bit-level equivalent of expression 4*f for
#   floating point argument f.
#   Both the argument and result are passed as unsigned ints, but
#   they are to be interpreted as the bit-level representation of
#   single-precision floating point values.
#   When argument is NaN, return argument
#   Rating: 4
.global floatScale4
floatScale4:
    movl %edi, %esi         # move uf into %esi
    shrl $31, %esi          # uf >> 31 (sign)
    
    movl %edi, %edx         # move uf into %edx
    shrl $23, %edx          # uf >> 23
    andl $0xFF, %edx        # (uf >> 23) & 0xFF (exp)
    
    movl %edi, %ecx         # move uf into %ecx
    andl $0x7FFFFF, %ecx    # uf & 7FFFFF (frac)

    cmpl $0, %edx           # if (exp == 0) jump
    je EQUALS             
    cmpl $0xFF, %edx        # else if (exp < 0xFF) jump
    jl ELIF
    jmp DONE                # jump to the end

ELIF:
    addl $2, %edx           # exp += 2
    cmpl $0xFF, %edx        # if (exp >= 0xFF) jump
    jge GREATERS           
    jmp DONE                # else jump to the end

EQUALS:
    shll $2, %ecx           # frac = frac << 2
    cmpl $0x7FFFFF, %ecx    # if (frac > 0x7FFFFF) jump
    jg EQUALS2
    jmp DONE                # else jump to the end

EQUALS2:
    movl $1, %edx               # exp = 1
    LOOP:
        movl $0xFFFFFF, %edi    # move 0xFFFFFF into %edi
        notl %edi               # ~0xFFFFFF
        andl %ecx, %edi         # frac & ~0xFFFFFF
        cmpl $0, %edi           # compare 0 and the above
        je EQUALS3              # jump out of the loop if zero
        shrl $1, %ecx           # frac >>= 1
        incl %edx               # exp++
        jmp LOOP                # jump to the top of the loop
EQUALS3:
    andl $0x7FFFFF, %ecx        # frac &= 0x7FFFFF
    jmp DONE                    # jump to the end

GREATERS:
    movl $0xFF, %edx       # exp = 0xFF
    movl $0, %ecx          # frac = 0
    jmp DONE                # jump to the end

DONE:
    shll $31, %esi          # sign << 31
    shll $23, %edx          # exp << 23
    orl %esi, %edx          # (sign << 31) | (exp << 23)
    orl %edx, %ecx          # or that above with frac
    movl %ecx, %eax         # move that value into the return register and return
    ret


# greatestBitPos - return a mask that marks the position of the
#               most significant 1 bit. If x == 0, return 0
#   Example: greatestBitPos(96) = 0x40
#   Rating: 4
.global greatestBitPos
greatestBitPos:
    movl %edi, %ecx     # move x into ecx
    movl %ecx, %edx     # move x into edx
    sarl $1, %ecx       # right shift 1 by x
    orl %ecx, %edx      # or that with x
    movl %edx, %ecx     # move that into ecx
    sarl $2, %ecx       # right shift the above by 2
    orl %ecx, %edx      # or that with x
    movl %edx, %ecx     # move that into ecx
    sarl $4, %ecx       # right shift the above by 4
    orl %ecx, %edx      # or that with x
    movl %edx, %ecx     # move that into ecx
    sarl $8, %ecx       # right shift the above by 8
    orl %ecx, %edx      # or that with x
    movl %edx, %ecx     # move that into ecx
    sarl $16, %ecx      # right shift the above by 16
    orl %ecx, %edx      # or that with x
    movl %edx, %ecx     # move that into ecx

    sarl $1, %ecx       # right shift that by 1
    notl %ecx           # not that
    movl $0x80000000, %eax      # move 0x80000000 into eax
    orl %ecx, %eax              # or that with the notted value
    andl %edx, %eax             # and that with x
    ret
