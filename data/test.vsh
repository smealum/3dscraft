; make sure you update aemstro_as for this (15/11/14)
 
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
		mov r1,  v0     (0x6)
		mov r1,  c5     (0x3)
		; tempreg = mdlvMtx * in.pos
		dp4 r0, c36, r1 (0x0)
		dp4 r0, c37, r1 (0x1)
		dp4 r0, c38, r1 (0x2)
		mov r0,  c5     (0x3)
		; result.pos = projMtx * tempreg
		dp4 o0, c32, r0 (0x0)
		dp4 o0, c33, r0 (0x1)
		dp4 o0, c34, r0 (0x2)
		dp4 o0, c35, r0 (0x3)
		; result.texcoord = in.texcoord
		mov o2, v1      (0x5)
		mov o3, c5      (0x7)
		mov o4, c5      (0x7)
		; result.color = crappy lighting
		dp3 r0, c40, v2 (0x6)
		max r0,  c5, r0 (0x4)
		mul r0, c41, r0 (0x8)
		add o1, c41, r0 (0x6)
		mov o1,  c5     (0x3)
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
