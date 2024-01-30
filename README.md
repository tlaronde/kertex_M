# kertex_M
Web to C conversion framework for the TeX system.

This is the `M' (Matrix) part of the kerTeX distribution since it is
only a compilation tool, obtaining the Pascal code from the WEB files,
and translating _this_ Pascal to C.

The main tool is called pp2rc for Pseudo-Pascal to Raw C, because the
procedure is ad hoc: this is not a general purpose Pascal to C
translation framework.

This part is never installed. It is used for compilation par kertex_T
(the `T' for Target part; the main distribution in fact).
