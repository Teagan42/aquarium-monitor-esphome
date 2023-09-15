#include "Matrix.h"
#include <cstring>

// Solve the matrix equation Ax = v for x; where x, v are column vectors
// will work on copies of A,v so originals are undisturbed
Matrix solveFor(Matrix A, Matrix v)
{
    if (A._row <= 0 || A._column != A._row || A._row != v._row ||
        v._column != 1)
    {
        //     throw BadSolveArgs();
        return Matrix();
    }

    for (int pivot = 0; pivot < A._row; pivot++)
    {
        // find best row for pivot - row with largest value in the pivot column
        float maxPivot = 0;
        int bestRow = -1;
        for (int chkRow = pivot; chkRow < A._row; chkRow++)
        {
            if (A._entity[chkRow][pivot] > maxPivot)
            {
                bestRow = chkRow;
                maxPivot = A._entity[chkRow][pivot];
            }
            else if (-A._entity[chkRow][pivot] > maxPivot)
            {
                bestRow = chkRow;
                maxPivot = -A._entity[chkRow][pivot];
            }
        }

        if (bestRow == -1)
        {
            // throw Singular(); //error
            return Matrix();
        }

        if (bestRow != pivot)
        {
            A.swapRow(pivot, bestRow);
            v.swapRow(pivot, bestRow);
        }

        maxPivot = A._entity[pivot][pivot]; // get the sign back if was -ve
        // we've got our pivot row - divide row by maxPivot to make pivot value 1
        A._entity[pivot][pivot] = 1;
        for (int c = pivot + 1; c < A._column; c++)
        {
            A._entity[pivot][c] /= maxPivot;
        }
        // and do same in vector
        v._entity[pivot][0] /= maxPivot;

        // we'll go full Gaussian on this - as it saves some coding
        // TODO could be more efficient?
        for (int r = 0; r < A._row;
             r++)
        { // full gaussian so do all rows but our own
            if (r != pivot)
            {
                float f = A._entity[r][pivot];
                A._entity[r][pivot] = 0;
                for (int c = pivot + 1; c < A._column; c++)
                {
                    A._entity[r][c] -= f * A._entity[pivot][c];
                }
                v._entity[r][0] -= f * v._entity[pivot][0];
            }
        }
    }

    return v;
}

Matrix inv(const Matrix &A)
{
    // has to be a square matrix
    if (A._row != A._column)
    {
        // esphome::ESP_LOGI("Inv Has to be a square matrix");
    }

    Matrix tmp(A._row, A._column, 'I');
    Matrix copy(A);

    // set the pivot to be the largest element in that column
    for (int i = 0; i < A._row - 1; i++)
    {
        float pivot = copy._entity[i][i];
        int row = i;
        for (int j = i + 1; j < A._row; j++)
            if (copy._entity[j][i] > pivot)
            {
                pivot = copy._entity[j][i];
                row = j;
            }

        if (pivot == 0)
        {
            return Matrix();
        }
        else
        {
            copy.swapRow(i, row);
            tmp.swapRow(i, row);
        }
    }

    for (int i = 0; i < A._row; i++)
        for (int j = 0; j < A._row; j++)
            if (i != j)
            {
                float backup = copy._entity[j][i];
                for (int k = 0; k < A._column; k++)
                {
                    copy._entity[j][k] -= backup * copy._entity[i][k] / copy._entity[i][i];
                    tmp._entity[j][k] -= backup * tmp._entity[i][k] / copy._entity[i][i];
                }
            }

    for (int i = 0; i < A._row; i++)
        for (int j = 0; j < A._column; j++)
        {
            tmp._entity[i][j] /= copy._entity[i][i];
        }

    return tmp;
}

Matrix transpose(const Matrix &A)
{
    Matrix tmp(A._column, A._row);

    for (int i = 0; i < A._row; i++)
        for (int j = 0; j < A._column; j++)
            tmp._entity[j][i] = A._entity[i][j];

    return tmp;
}

Matrix::Matrix()
{
    _row = 0;
    _column = 0;
    _entity = nullptr;
}

Matrix::Matrix(int r, int c, int ini)
{
    _row = r;
    _column = c;
    _entity = new float *[_row];
    for (int i = 0; i < _row; i++)
        _entity[i] = new float[_column];

    for (int i = 0; i < _row; i++)
        for (int j = 0; j < _column; j++)
            _entity[i][j] = ini;
}

Matrix::Matrix(const Matrix &m)
{
    _row = m._row;
    _column = m._column;

    _entity = new float *[_row];
    for (int i = 0; i < _row; i++)
        _entity[i] = new float[_column];

    int bytes = _column * sizeof(float);
    for (int i = 0; i < _row; i++)
        memcpy(_entity[i], m._entity[i], bytes);
}

Matrix::Matrix(Matrix &&m)
{
    _row = m._row;
    _column = m._column;
    _entity = m._entity;
    m._entity = nullptr;
    m._row = 0;
    m._column = 0;
}

Matrix::Matrix(int r, int c, float *m)
{
    _row = r;
    _column = c;

    _entity = new float *[_row];
    for (int i = 0; i < r; i++)
        _entity[i] = new float[_column];

    int bytes = _column * sizeof(float);
    for (int i = 0; i < _row; i++)
        memcpy(_entity[i], m + i * _column, bytes);
}

Matrix::Matrix(int r, int c, char type)
{
    _row = r;
    _column = c;

    _entity = new float *[_row];
    for (int i = 0; i < r; i++)
        _entity[i] = new float[_column];

    switch (type)
    {
    case 'I': // identity matrix
        if (r == c)
        {
            for (int i = 0; i < _row; i++)
                for (int j = 0; j < _column; j++)
                    _entity[i][j] = (i == j ? 1 : 0);
        }
        else
        {
            if (_entity != nullptr)
            {
                for (int i = 0; i < _row; i++)
                    delete[] _entity[i];
                delete[] _entity;
            }
        }
        break;
    default:
        for (int i = 0; i < _row; i++)
            for (int j = 0; j < _column; j++)
                _entity[i][j] = 0;
    }
}

Matrix::~Matrix()
{
    if (_entity != nullptr)
    {
        for (int i = 0; i < _row; i++)
            delete[] _entity[i];
        delete[] _entity;
    }
}

Matrix Matrix::operator*(const Matrix &A)
{
    // if (_column != A._row)
    //     esphome::ESP_LOGI("Multiplication matrix dimension not match");

    Matrix tmp(_row, A._column);

    for (int i = 0; i < _row; i++)
        for (int j = 0; j < A._column; j++)
        {
            for (int k = 0; k < _column; k++)
                tmp._entity[i][j] += _entity[i][k] * A._entity[k][j];
        }

    return tmp;
}

Matrix &Matrix::operator*=(const Matrix &A)
{
    // check if the dimension matches
    if (_column != A._row)
    {
        if (_entity != nullptr)
        {
            for (int i = 0; i < _row; i++)
                delete[] _entity[i];
            delete[] _entity;
        }
        return *this;
    }

    Matrix tmp(*this);

    // the dimension of the result matrix does not match the calling matrix
    if (_column != A._column)
    {
        for (int i = 0; i < _row; i++)
            delete[] _entity[i];

        _column = A._column;

        for (int i = 0; i < _row; i++)
            _entity[i] = new float[_column];

        for (int i = 0; i < _row; i++)
            for (int j = 0; j < _column; j++)
            {
                _entity[i][j] = 0;
                for (int k = 0; k < tmp._column; k++)
                    _entity[i][j] += tmp._entity[i][k] * A._entity[k][j];
            }
    }
    else
    { // the dimension of the result matrix matches the calling matrix
        for (int i = 0; i < _row; i++)
            for (int j = 0; j < _column; j++)
            {
                _entity[i][j] = 0;
                for (int k = 0; k < _column; k++)
                    _entity[i][j] += tmp._entity[i][k] * A._entity[k][j];
            }
    }

    return *this;
}

Matrix Matrix::operator*(float a)
{
    Matrix tmp(_row, _column);

    for (int i = 0; i < _row; i++)
        for (int j = 0; j < _column; j++)
        {
            tmp._entity[i][j] = _entity[i][j] * a;
        }

    return tmp;
}

Matrix &Matrix::operator*=(float a)
{
    for (int i = 0; i < _row; i++)
        for (int j = 0; j < _column; j++)
        {
            _entity[i][j] *= a;
        }

    return *this;
}

template <class G>
Matrix operator*(G a, const Matrix &A)
{
    return A * a;
}

Matrix Matrix::operator/(float a)
{
    Matrix tmp(_row, _column);

    for (int i = 0; i < _row; i++)
        for (int j = 0; j < _column; j++)
        {
            tmp._entity[i][j] = _entity[i][j] / a;
        }

    return tmp;
}

Matrix &Matrix::operator/=(float a)
{
    for (int i = 0; i < _row; i++)
        for (int j = 0; j < _column; j++)
        {
            _entity[i][j] /= a;
        }

    return *this;
}

Matrix Matrix::operator+(const Matrix &A)
{
    // if (_row != A._row || _column != A._column)
    //     esphome::ESP_LOGI("Plus matrix dimension not match");

    Matrix tmp(_row, _column);

    for (int i = 0; i < _row; i++)
        for (int j = 0; j < _column; j++)
        {
            tmp._entity[i][j] = _entity[i][j] + A._entity[i][j];
        }

    return tmp;
}

float Matrix::operator+(float a)
{
    // if (_row != 1 || _column != 1)
    //     esphome::ESP_LOGI("Plus scalar dimension not match");

    return _entity[0][0] + a;
}

template <class T>
T operator+(T a, const Matrix &A)
{
    // if (A._row != 1 || A._column != 1)
    //     esphome::ESP_LOGI("Plus scalar dimension not match");

    return a + A._entity[0][0];
}

Matrix &Matrix::operator+=(const Matrix &A)
{
    if (_row != A._row || _column != A._column)
    {
        for (int i = 0; i < _row; i++)
            delete[] _entity[i];
        delete[] _entity;
        return *this;
    }

    for (int i = 0; i < _row; i++)
        for (int j = 0; j < _column; j++)
        {
            _entity[i][j] = _entity[i][j] + A._entity[i][j];
        }

    return *this;
}

Matrix Matrix::operator-(const Matrix &A)
{
    // if (_row != A._row || _column != A._column)
    //     esphome::ESP_LOGI("Sub scalar dimension not match");

    Matrix tmp(_row, _column);

    for (int i = 0; i < _row; i++)
        for (int j = 0; j < _column; j++)
        {
            tmp._entity[i][j] = _entity[i][j] - A._entity[i][j];
        }

    return tmp;
}

float Matrix::operator-(float a)
{
    // if (_row != 1 || _column != 1)
    //     esphome::ESP_LOGI("Sub scalar dimension not match");

    return _entity[0][0] - a;
}

float operator-(float a, const Matrix &A)
{
    // if (A._row != 1 || A._column != 1)
    //     esphome::ESP_LOGI("Sub scalar dimension not match");

    return a - A._entity[0][0];
}

Matrix &Matrix::operator-=(const Matrix &A)
{
    if (_row != A._row || _column != A._column)
    {
        for (int i = 0; i < _row; i++)
            delete[] _entity[i];
        delete[] _entity;
        return *this;
    }

    for (int i = 0; i < _row; i++)
        for (int j = 0; j < _column; j++)
        {
            _entity[i][j] = _entity[i][j] - A._entity[i][j];
        }

    return *this;
}

float *Matrix::operator[](int index)
{
    return _entity[index];
}

Matrix &Matrix::operator=(const Matrix &A)
{
    if (_entity != nullptr)
    {
        for (int i = 0; i < _row; i++)
            delete[] _entity[i];
        delete[] _entity;
    }

    _row = A._row;
    _column = A._column;

    _entity = new float *[_row];
    for (int i = 0; i < _row; i++)
        _entity[i] = new float[_column];

    for (int i = 0; i < _row; i++)
        for (int j = 0; j < _column; j++)
            _entity[i][j] = A._entity[i][j];

    return *this;
}

Matrix &Matrix::operator=(Matrix &&A)
{
    if (_entity != nullptr)
    {
        for (int i = 0; i < _row; i++)
            delete[] _entity[i];
        delete[] _entity;
    }
    _row = A._row;
    _column = A._column;
    _entity = A._entity;
    A._entity = nullptr;
    A._row = 0;
    A._column = 0;
    return *this;
}

bool Matrix::operator!=(const Matrix &A)
{
    if (_row != A._row || _column != A._column)
        return true;

    for (int i = 0; i < _row; i++)
        for (int j = 0; j < _column; j++)
            if (_entity[i][j] != A._entity[i][j])
                return true;

    return false;
}

bool Matrix::operator==(const Matrix &A)
{
    if (_row != A._row || _column != A._column)
        return false;

    for (int i = 0; i < _row; i++)
        for (int j = 0; j < _column; j++)
            if (_entity[i][j] != A._entity[i][j])
                return false;

    return true;
}

void Matrix::swapRow(int i, int j)
{
    if (i != j && i < _row && j < _row && i >= 0 && j >= 0)
    {
        float tmp;
        for (int k = 0; k < _column; k++)
        {
            tmp = _entity[j][k];
            _entity[j][k] = _entity[i][k];
            _entity[i][k] = tmp;
        }
    }
}

void Matrix::show(int decimal)
{
    if (decimal <= 0)
    {
        for (int i = 0; i < _row; i++)
        {
            // Serial.print("[");
            for (int j = 0; j < _column; j++)
            {
                // Serial.print("  ");
                // Serial.print(_entity[i][j]);
                // Serial.print(" ");
            }
            // esphome::ESP_LOGI("]");
        }
        // esphome::ESP_LOGI();
    }
    else
    {
        for (int i = 0; i < _row; i++)
        {
            // Serial.print("[");
            for (int j = 0; j < _column; j++)
            {
                // Serial.print("  ");
                // Serial.print(_entity[i][j], decimal);
                // Serial.print(" ");
            }
            // esphome::ESP_LOGI("]");
        }
        // esphome::ESP_LOGI();
    }
}

bool Matrix::notEmpty()
{
    if (_entity == nullptr)
        return false;
    return true;
}