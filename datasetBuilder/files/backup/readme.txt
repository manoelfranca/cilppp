Changes of this version (20120219):

IMPROVEMENTS
	- Net: 
		* New parameter: HIT_MARGIN (defines what is considered a hit during training) (done)
		* Amin is not used anymore to the training stopping criterias (done)
		* Stochastic evaluation has been shut off from training (done)
		* Stochastic evaluation does not let any 'unknows' happens anymore during test
			(if evaluation fails, it turns to the opposite activation) (done)
			
	- Log:
		* Amin corrections and normalizations are being printed (done)
		* Test set error is now being printed also, pre-formatted for plotting (done)
		* All 3 possible kinds of tests (stable stochastic, stable non-stochastic, 
			non-stable stochastic, non-stable non-stochastic) are now presented (done)