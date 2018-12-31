# Knight-Tour-
Based on Warnsdorf's rules

This program works for any N x N boards where N = (5..31).
I chose to limit the size to 31 x 31 to allow full printout of the board. 
Starting point are selected randomly.
The algorithm is recursive, yet, there are some starting points where the program fails to find a complete tour.
For example for a N X N where N is odd the program fails to find a complete tour.

This program make use of internal library to allow formated number printout (nfrmt)- I needed that to print the total number of move attempts for those cases where ehither there was no solution or those that required several back tracks in order to find the right tour. this can be avoid by taking out the call for formating printout.


