

# schedule


# g

schedule->execute->gogo->{settls,switch ctx} // runnalbe->running

newproc->newproc1 // _Gidle->runnable
gopark->mcall(park_m) -> // runing->wait
mstart->mstart1->   // 
goexit0->                           
Gosched->mcall(gosched_m)->goschedImpl->

# m
wakep->startm->