#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <list>
#include <time.h>

using namespace std;

//Miscelaneous usefull functions and procedures
#pragma region Util
// Multiply a matrix by a vector and return the result
double *matrixVectorMultiplication(double **matrix, double *vec, int rowCountMatrix, int columnCountMatrix, int rowCountVector)
{
    double *result =  (double*) calloc(rowCountMatrix, sizeof(double));
    double sum;
    if (columnCountMatrix == rowCountVector + 1)
    {
        for (int i = 0; i < rowCountMatrix; i++)
        {
            sum = 0;
            #pragma omp parallel for reduction(+: sum)
            for (int j = 0; j < rowCountVector; j++)
            {
                sum += matrix[i][j] * vec[j];
            }
            result[i] = sum - matrix[i][rowCountVector];
        }
    }
    return result;
}

// Multiply a vector by a matrix and return the result
void vectorMatrixMultiplication(double *result, double *vec, double **matrix, int columnCountVector, int rowCountMatrix, int columnCountMatrix)
{
    double sum;
    if (columnCountVector == rowCountMatrix)
    {
        for (int i = 0; i < columnCountMatrix; i++)
        {
            sum = 0;
            #pragma omp parallel for reduction(+: sum)
            for (int j = 0; j < columnCountVector; j++)
            {
                sum += vec[j] * matrix[j][i];
            }
            result[i] = sum;
        }
    }
}

//prints a vector
void printVector(double *vector, int n, const char *message)
{
    cout << message << ": ";
    for(int i = 0; i < n; i++)
    {
        cout << vector[i] << " ";
    }
    cout << endl;
}

//prints a matrix
void printMatrix(double **matrix, int nRow, int nCol, const char *message)
{
    cout << message << ":\n";
    for(int i = 0; i < nRow; i++)
    {
        for(int j = 0; j < nCol; j++)
        {
            cout << matrix[i][j] << " ";
        }
        cout << endl;
    }
    cout << endl;
}
#pragma endregion

// Representation of the values with a given input at each step in the forward motion
struct State
{
    double *fNetHidden; // Stores the values given by the neurons on the hidden layer
    double *fNetOutput; // Stores the values given by the neurons on the output layer
} typedef State;

// Multi-Layer Perceptron neural network class
class MLP
{
#pragma region Properties
private:
    int inputLength;       // Number of inputs for each neuron
    int hiddenLayerLength; // Number of neurons in the hidden layer
    int outputLayerLength; // Number of neurons in the output layer

    double **hiddenLayer; // Array storing weights and thresholds of each neuron in the hidden layer
    double **outputLayer; // Array storing weights and thresholds of each neuron in the output layer

#pragma endregion

public:
#pragma region Constructor
    MLP()
    {
        this->inputLength = 2;
        this->hiddenLayerLength = 2;
        this->outputLayerLength = 1;

        this->hiddenLayer = (double **)calloc(this->hiddenLayerLength, sizeof(double *));
        for (int i = 0; i < this->hiddenLayerLength; i++)
        {
            this->hiddenLayer[i] = (double *)calloc(this->inputLength + 1, sizeof(double));
        }

        this->outputLayer = (double **)calloc(this->outputLayerLength, sizeof(double *));
        for (int i = 0; i < this->outputLayerLength; i++)
        {
            this->outputLayer[i] = (double *)calloc(this->hiddenLayerLength + 1, sizeof(double));
        }

        this->populateNeurons();
    }

    MLP(int inputLength, int hiddenLayerLength, int outputLayerLength)
    {
        this->inputLength = inputLength;
        this->hiddenLayerLength = hiddenLayerLength;
        this->outputLayerLength = outputLayerLength;

        this->hiddenLayer = (double **)calloc(this->hiddenLayerLength, sizeof(double *));
        for (int i = 0; i < this->hiddenLayerLength; i++)
        {
            this->hiddenLayer[i] = (double *)calloc(this->inputLength + 1, sizeof(double));
        }

        this->outputLayer = (double **)calloc(this->outputLayerLength, sizeof(double *));
        for (int i = 0; i < this->outputLayerLength; i++)
        {
            this->outputLayer[i] = (double *)calloc(this->hiddenLayerLength + 1, sizeof(double));
        }

        this->populateNeurons();
    }
#pragma endregion

#pragma region Methods
    // Initialize the values for weights and thresholds for each neuron
    void populateNeurons()
    {
        srand(time(NULL));

        for (int i = 0; i < this->hiddenLayerLength; i++)
        {
            for (int j = 0; j < this->inputLength + 1; j++)
            {
                // Generates a random number between -0.5 and 0.5 for each weight and threshold of a neuron
                this->hiddenLayer[i][j] = ((double)rand() / (RAND_MAX)) - 0.5;
            }
        }

        for (int i = 0; i < this->outputLayerLength; i++)
        {
            for (int j = 0; j < this->hiddenLayerLength + 1; j++)
            {
                // Generates a random number between -0.5 and 0.5 for each weight and threshold of a neuron
                this->outputLayer[i][j] = ((double)rand() / (RAND_MAX)) - 0.5;
            }
        }
    }

    // Given the weighted values received by the neuron, this function is executed in order for the neuron to find a result
    void activation(double *net, int netLength)
    {
        for (int i = 0; i < netLength; i++)
        {
            net[i] = 1.0 / (1 + exp(-net[i]));
        }
    }

    // Calculates the derivatives of the activated values and places the result in df_dnet
    void dNet(double *df_dnet, double *f_net, int f_netLength)
    {
        for (int i = 0; i < f_netLength; i++)
        {
            df_dnet[i] = (f_net[i] * (1 - f_net[i]));
        }
    }

    // Receives one input from the dataset and obtains a response from the network
    State forward(double *input)
    {
        double *netHiddenLayer, *netOutputLayer;
        // Finds the net value (sum of weighted inputs) received by the neuron in the hidden layer
        netHiddenLayer = matrixVectorMultiplication(this->hiddenLayer, input, this->hiddenLayerLength, this->inputLength + 1, this->inputLength);

        //printVector(netHiddenLayer, this->hiddenLayerLength, "netH");
        
        // Finds the values calculated by the hidden layer
        this->activation(netHiddenLayer, this->hiddenLayerLength);

        //printVector(netHiddenLayer, this->hiddenLayerLength, "fNetH");
        
        // Output layer

        // Finds the net value (sum of weighted inputs) received by the neuron in the output layer
        netOutputLayer = matrixVectorMultiplication(this->outputLayer, netHiddenLayer, this->outputLayerLength, this->hiddenLayerLength + 1, this->hiddenLayerLength);

        //printVector(netOutputLayer, this->outputLayerLength, "netO");
        
        // Finds the values calculated by the hidden layer
        this->activation(netOutputLayer, this->outputLayerLength);

        //printVector(netOutputLayer, this->outputLayerLength, "fNetO");
        
        // Declares and instantiates the current state of the network
        State state;
        state.fNetHidden = netHiddenLayer;
        state.fNetOutput = netOutputLayer;

        return state;
    }

    // Trains the neural network based on the mistakes made
    void backPropagation(double **input, double **output, int datasetLength, double trainingRate, double threshold)
    {
        int count = 0;
        // Vectors used in the method
        double *Xp, *Yp, *errors, *dNetOutput, *deltaOutput, *dNetHidden, *deltaHidden, *outputDerivative;
        // Matrices used in the method
        //double /**outputLayerCorrection,*/ **hiddenLayerCorrection;
        State results;

        //Allocates memory for the arrays
        // Declares a vector for the calculated errors
        errors = (double *)calloc(this->outputLayerLength, sizeof(double));
        // Declares the vector for the line derivative that represents the error for the output layer
        dNetOutput = (double *)calloc(this->outputLayerLength, sizeof(double));
        // Declares a vector for the calculated derivatives
        deltaOutput = (double *)calloc(this->outputLayerLength, sizeof(double));
        outputDerivative = (double *)calloc(this->hiddenLayerLength, sizeof(double));

        // Declares the vector for the line derivative that represents the error for the hidden layer
        dNetHidden = (double *)calloc(this->hiddenLayerLength, sizeof(double));
        // Declares a vector for the calculated derivatives
        deltaHidden = (double *)calloc(this->hiddenLayerLength, sizeof(double));

        double squaredError = 2 * threshold;
        // Executes the loop while the error acceptance is not satiated
        while (squaredError > threshold && count < 10000)
        {
            squaredError = 0;

            for (int p = 0; p < datasetLength; p++)
            {
                //printMatrix(outputLayer, this->outputLayerLength, this->hiddenLayerLength + 1, "Output Layer");

                //printMatrix(hiddenLayer, this->hiddenLayerLength, this->inputLength + 1, "Hidden Layer");
                // Extracts input pattern
                Xp = input[p];
                // Extracts output pattern
                Yp = output[p];

                //printVector(Xp, this->inputLength, "Xp");
                //printVector(Yp, this->outputLayerLength, "Yp");

                // Obtains the results given by the network
                results = this->forward(Xp);

                //printVector(results.fNetHidden, this->hiddenLayerLength, "Results fNetH");

                //printVector(results.fNetOutput, this->outputLayerLength, "Results fNetO");

#pragma region Output layer manipulation
                
                // Calculates the error for each value obtained
                for (int i = 0; i < this->outputLayerLength; i++)
                {
                    errors[i] = Yp[i] - results.fNetOutput[i];
                    squaredError += pow(errors[i], 2);
                }

                //printVector(errors, this->outputLayerLength, "Errors");

                // Finds the derivative of the line that represents the error
                dNet(dNetOutput, results.fNetOutput, this->outputLayerLength);

                //printVector(dNetOutput, this->outputLayerLength, "dNetO");
                
                // Calculates the derivative for each error stored
                for (int i = 0; i < this->outputLayerLength; i++)
                {
                    deltaOutput[i] = errors[i] * dNetOutput[i];
                }

                //printVector(deltaOutput, this->outputLayerLength, "deltaOutput");
#pragma endregion

#pragma region Hidden layer manipulation
                vectorMatrixMultiplication(outputDerivative, deltaOutput, this->outputLayer, this->outputLayerLength, this->outputLayerLength, this->hiddenLayerLength);

                //printVector(outputDerivative, this->hiddenLayerLength, "outDeriv");
                
                // Finds the derivative of the line that represents the error
                dNet(dNetHidden, results.fNetHidden, this->hiddenLayerLength);

                //printVector(dNetHidden, this->hiddenLayerLength, "dNetH");
                
                // Calculates the derivative for each error stored
                for (int i = 0; i < this->hiddenLayerLength; i++)
                {
                    deltaHidden[i] = outputDerivative[i] * dNetHidden[i];
                }

                //printVector(deltaHidden, this->hiddenLayerLength, "deltaH");
#pragma endregion

#pragma region Effective training

                for (int i = 0; i < this->outputLayerLength; i++)
                {
                    for (int j = 0; j < this->hiddenLayerLength; j++)
                    {
                        this->outputLayer[i][j] += trainingRate * deltaOutput[i] * results.fNetHidden[j];
                    }
                    this->outputLayer[i][hiddenLayerLength] -= trainingRate * deltaOutput[i];
                }

                for (int i = 0; i < this->hiddenLayerLength; i++)
                {
                    for (int j = 0; j < this->inputLength; j++)
                    {
                        this->hiddenLayer[i][j] += trainingRate * deltaHidden[i] * Xp[j];
                    }
                    this->hiddenLayer[i][inputLength] -= trainingRate * deltaHidden[i];
                }
#pragma endregion
            }

            squaredError /= datasetLength;
            count++;
	
	        //cout << "squaredError = " << squaredError << endl << "Count = " << count << endl << endl;
        }
        // Clear all used memory
        delete errors;
        delete deltaOutput;
        delete deltaHidden;
        delete dNetOutput;
        delete dNetHidden;
        delete outputDerivative;
        

        //cout << "Number of epochs = " << count << endl;
    }

#pragma endregion
};

int main(int argc, char *argv[])
{
    // Declares and reads the main values of the dataset input
    int datasetLength, inputLength, outputLength, hiddenLength = atoi(argv[1]);
    double trainingRate = atof(argv[2]), threshold = atof(argv[3]);

    cin >> datasetLength >> inputLength >> outputLength;

    // Alocates memory for the dataset reading
    double **input = (double **)calloc(datasetLength, sizeof(double *));
    double **output = (double **)calloc(datasetLength, sizeof(double *));

    // Reads the dataset data
    for (int i = 0; i < datasetLength; i++)
    {
        input[i] = (double *)calloc(inputLength, sizeof(double));
        output[i] = (double *)calloc(outputLength, sizeof(double));

        for (int j = 0; j < inputLength; j++)
        {
            cin >> input[i][j];
        }

        for (int j = 0; j < outputLength; j++)
        {
            cin >> output[i][j];
        }
    }

    // Declares the MLP network class
    MLP *mlp = new MLP(inputLength, hiddenLength, outputLength);

    // Executes the neural network training
    mlp->backPropagation(input, output, datasetLength, trainingRate, threshold);


    #pragma region Testing
    State state;

    double *Xp;

    int errorCount = 0;
    
    /*for(int i = datasetLength/2; i < datasetLength; i++)
    {
        Xp = input[i];
        state = mlp->forward(Xp);
        //cout << "Test " << (i+1) << ":" << endl;
        //cout << "\tExpected: " << output[i][0] << endl;
        //cout << "\tObtained: " << state.fNetOutput[0] << '\n';
        if(output[i][0] != round(state.fNetOutput[0])) errorCount++;
        //cout << round(state.fNetOutput[0]) << '\n';
    }*/

    for(int i = 0; i < datasetLength; i++)
    {
        Xp = input[i];
        state = mlp->forward(Xp);
        //cout << "Test " << (i+1) << ":" << endl;
        //cout << "\tExpected: " << output[i][0] << endl;
        //cout << "\tObtained: " << state.fNetOutput[0] << '\n';
        if(output[i][0] != round(state.fNetOutput[0])) errorCount++;
        //cout << round(state.fNetOutput[0]) << '\n';
    }

    int nTests = datasetLength;
    cout << "Number of tests = " << nTests << endl;
    int nSucc = nTests - errorCount;
    cout << "Number of succesful answers = " << nSucc << endl;
    cout << "Number of errors = " << errorCount << endl;
    cout << "Accuracy = " << (double)nSucc/nTests * 100 << "%\n"; 
    #pragma endregion

    return (0);
}
