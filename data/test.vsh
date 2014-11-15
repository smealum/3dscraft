; make sure you update aemstro_as for this (27/05/14)
 
; setup constants
	.const 5, 1.0, 0.0, 0.5, 1.0
 
; setup outmap
	.out o0, result.position
	.out o1, result.color
	.out o2, result.texcoord0
	.out o3, result.texcoord1
	.out o4, result.texcoord2
 
;code
	main:
		mov d1A, d00 (0x6)
		mov d1A, d25 (0x3)
		; tempreg = mdlvMtx * in.pos
		dp4 d10, d44, d1A (0x0)
		dp4 d10, d45, d1A (0x1)
		dp4 d10, d46, d1A (0x2)
		mov d10, d25 (0x3)
		; result.pos = projMtx * tempreg
		dp4 d00, d40, d10 (0x0)
		dp4 d00, d41, d10 (0x1)
		dp4 d00, d42, d10 (0x2)
		dp4 d00, d43, d10 (0x3)
		; result.texcoord = in.texcoord
		mov d02, d01 (0x5)
		mov d03, d25 (0x7)
		mov d04, d25 (0x7)
		; result.color = crappy lighting
		dp3 d10, d48, d02 (0x6)
		max d10, d25, d10 (0x4)
		mul d10, d49, d10 (0x8)
		add d01, d49, d10 (0x6)
		mov d01, d25 (0x3)
		flush
		end
	endmain:
 
;operand descriptors
	.opdesc x___, xyzw, xyzw ; 0x0
	.opdesc _y__, xyzw, xyzw ; 0x1
	.opdesc __z_, xyzw, xyzw ; 0x2
	.opdesc ___w, xyzw, xyzw ; 0x3
	.opdesc xyz_, yyyy, xyzw ; 0x4
	.opdesc xyzw, xyzw, xyzw ; 0x5
	.opdesc xyz_, xyzw, xyzw ; 0x6
	.opdesc xyzw, yyyw, xyzw ; 0x7
	.opdesc xyz_, wwww, xyzw ; 0x8
