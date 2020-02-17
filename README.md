# thread_c_multiplier
This is a project for the university. Its main goal is to teach us how to use concurrency and how to synchronize our programs.
we have to make a product of matrices.
Each cell of the resulting matrix must be calculated using a thread.
 Thus we are left with (n * m) threads to manage in the case of a resulting matrix which has n rows and m columns.

To carry out this project, I implemented a structure which has the list of all the matrices which one reads either starting from the standard entry or starting from a file using mmap.

Each matrix has a number of columns and rows and then a set of vectors representing its values.

This is how the program basically works.

# How to calculate the resulting matrix
We have two matrixes given, we just have to calculate their product and then assign it to the resulting matrix.
There are a lot of complex operations here.

At the beginning of the program, we kinda convert a text file to a <b>struct p_informations_t</b> structure which holds
the number of products we'll need to calculate and a list of <b>struct matrice_t</b> structures which represents a matrix.

We then verify if the operation can be done by checking if the first matrix's number of columns equal to the second matrix's number of rows,
if equals we initialize the resulting matrix and start the operations.

# Each operation equals to one thread
To do an operation, we need to pass some arguments to the threads that will handle it.
Therefore I created a <b>struct thread_matrice_params_t</b> which contains the necesseary datas such the rows and the columns
that are required for the operations, two indices (x & y) that represents where the result should be written on the result matrix
vectors (two dimensionnal array) and an ID to help us check if the current thread has finished its task.

We then run all the threads required and pass them the required arguments.
It will execute the "mult" function.

# How does the mult function works ?
The mult function will receive the data and then cast it to a usable struct.
If the mult operation is used by another thread we'll have to wait until it's free then we lock it.
We then have to calculate the product of the matrix for one case of the final resulting matrix. If we're done we pass the value <b>0</b>
 to the pending operations to tell the <b>struct Product</b> that this thread is done and ainsi it'll decrement the operations
 that remain to be done.
 If the operation is the last one, we then pass the state of the application to SAVE.
 
 We save the resulting matrix and we're done.
