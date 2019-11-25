# goc  
[![workflow](https://github.com/jingyugao/tco/workflows/C/C++%20CI/badge.svg)]( https://github.com/jingyugao/tco/workflows/C/C++%20CI/badge.svg)

Goroutine implement of c.

## start 
runtime.rt0_go->runtime.main->main_main(user main).

## exit
retq->goexit->goexit1->goexit0.
