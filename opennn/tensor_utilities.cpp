//   OpenNN: Open Neural Networks Library
//   www.opennn.net
//
//   T E N S O R   U T I L I T I E S   S O U R C E
//
//   Artificial Intelligence Techniques, SL
//   artelnics@artelnics.com

#include "tensor_utilities.h"

#define GET_VARIABLE_NAME(Variable) (#Variable)

namespace opennn
{

void update_progress_bar(type progress) {
    const int barWidth = 50;

    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << std::setw(3) << static_cast<int>(progress * 100.0) << "%\r";
    std::cout.flush();
}

void initialize_sequential(Tensor<type, 1>& vector)
{
    for(Index i = 0; i < vector.size(); i++) vector(i) = type(i);
}


void initialize_sequential(Tensor<Index, 1>& vector)
{
    for(Index i = 0; i < vector.size(); i++) vector(i) = i;
}


void initialize_sequential(Tensor<Index, 1>& new_tensor,
                           const Index& start, const Index& step, const Index& end)
{
    const Index new_size = (end-start)/step+1;

    new_tensor.resize(new_size);
    new_tensor(0) = start;

    for(Index i = 1; i < new_size-1; i++)
    {
        new_tensor(i) = new_tensor(i-1)+step;
    }

    new_tensor(new_size-1) = end;
}


void multiply_rows(Tensor<type, 2>& matrix, const Tensor<type, 1>& vector)
{
    const Index columns_number = matrix.dimension(1);
    const Index rows_number = matrix.dimension(0);

    #pragma omp parallel for

    for(Index i = 0; i < rows_number; i++)
    {
        for(Index j = 0; j < columns_number; j++)
        {
           matrix(i,j) *= vector(j);
        }
    }
}


void divide_columns(ThreadPoolDevice* thread_pool_device, Tensor<type, 2>& matrix, const Tensor<type, 1>& vector)
{
    const Index rows_number = matrix.dimension(0);
    const Index columns_number = matrix.dimension(1);

    for(Index j = 0; j < columns_number; j++)
    {
        TensorMap<Tensor<type,1>> column(matrix.data() + j*rows_number, rows_number);

        column.device(*thread_pool_device) = column/vector(j);
    }
}


void divide_columns(ThreadPoolDevice* thread_pool_device, TensorMap<Tensor<type, 2>>& matrix, const Tensor<type, 1>& vector)
{
    const Index rows_number = matrix.dimension(0);
    const Index columns_number = matrix.dimension(1);

    for(Index i = 0; i < columns_number; i++)
    {
        TensorMap<Tensor<type,1>> column(matrix.data() + i*rows_number, rows_number);
        column.device(*thread_pool_device) = column / vector;
    }
}


bool is_zero(const Tensor<type, 1>& tensor)
{
    const Index size = tensor.size();

    for(Index i = 0; i < size; i++)
    {
        if(abs(tensor[i]) > type(NUMERIC_LIMITS_MIN)) return false;
    }

    return true;
}


bool is_zero(const Tensor<type,1>& tensor,const type& limit)
{
    const Index size = tensor.size();

    for(Index i = 0; i < size; i++)
    {
        if(abs(tensor[i]) > type(limit)) return false;
    }

    return true;
}


bool is_nan(const Tensor<type,1>& tensor)
{
    const Index size = tensor.size();

    for(Index i = 0; i < size; i++)
    {
        if(!isnan(tensor[i])) return false;
    }

    return true;
}


bool is_nan(const type& value)
{

    if(!isnan(value)) return false;

    return true;
}


bool is_false(const Tensor<bool, 1>& tensor)
{
    const Index size = tensor.size();

    for(Index i = 0; i < size; i++)
    {
        if(tensor(i)) return false;
    }

    return true;
}

Index true_count(const Tensor<bool, 1>& tensor)
{
    Index trueCount = 0;

    for (int i = 0; i < tensor.size(); ++i)
    {
        if(tensor(i))
        {
            trueCount++;
        }
    }

    return trueCount;
}


bool is_binary(const Tensor<type, 2>& matrix)
{
    const Index size = matrix.size();

    for(Index i = 0; i < size; i++)
    {
        if(matrix(i) != type(0) && matrix(i) != type(1) && !isnan(matrix(i)) ) return false;
    }

    return true;
}


bool is_constant(const Tensor<type, 1>& vector)
{
    const Index size = vector.size();

    type first_not_nan_element = 0.0;

    for(Index i = 0; i < size; i++)
    {
        if(isnan(vector(i)))
        {
            continue;
        }
        else
        {
            first_not_nan_element = vector(i);
            break;
        }
    }

    for(Index i = 0; i < size; i++)
    {
        if(isnan(vector(i))) continue;

        if(abs(first_not_nan_element - vector(i)) > numeric_limits<float>::min()) return false;
    }

    return true;
}


bool is_equal(const Tensor<type, 2>& matrix, const type& value, const type& tolerance)
{
    const Index size = matrix.size();

    for(Index i = 0; i < size; i++)
    {
        if(abs(matrix(i) - value) > tolerance) return false;
    }

    return true;
}


bool are_equal(const Tensor<type, 1>& vector_1, const Tensor<type, 1>& vector_2, const type& tolerance)
{
    const Index size = vector_1.size();

    for(Index i = 0; i < size; i++)
    {
        if(abs(vector_1(i) - vector_2(i)) > tolerance) return false;
    }

    return true;
}


bool are_equal(const Tensor<bool, 1>& vector_1, const Tensor<bool, 1>& vector_2)
{
    const Index size = vector_1.size();

    for(Index i = 0; i < size; i++)
    {
        if( vector_1(i) != vector_2(i)) return false;
    }

    return true;
}


bool are_equal(const Tensor<type, 2>& matrix_1, const Tensor<type, 2>& matrix_2, const type& tolerance)
{
    const Index size = matrix_1.size();

    for(Index i = 0; i < size; i++)
    {
        if(abs(matrix_1(i) - matrix_2(i)) > tolerance) return false;
    }

    return true;
}

bool are_equal(const Tensor<bool, 2>& matrix_1, const Tensor<bool, 2>& matrix_2)
{
    const Index size = matrix_1.size();

    for(Index i = 0; i < size; i++)
    {
        if( matrix_1(i) != matrix_2(i)) return false;
    }

    return true;
}


Tensor<bool, 2> elements_are_equal(const Tensor<type, 2>& x, const Tensor<type, 2>& y)
{
    if(x.size() != y.size() || x.dimension(0) != y.dimension(0) || x.dimension(1) != y.dimension(1))
    {
        ostringstream buffer;

        buffer << "OpenNN Exception: Tensor utilities class.\n"
            << "Tensor<bool, 2> elements_are_equal(const Tensor<type, 2>& x, const Tensor<type, 2>& y) method.\n"
            << "Input vectors must have equal sizes.\n";

        throw invalid_argument(buffer.str());
    }

    Tensor<bool, 2> result(x.dimension(0), x.dimension(1));

    for(int i = 0; i < x.size(); i++) { result(i) = (x(i) == y(i)); };

    return result;
}

void save_csv(const Tensor<type,2>& data, const string& filename)
{
    ofstream file(filename);

    if(!file.is_open())
    {
      ostringstream buffer;

      buffer << "OpenNN Exception: Matrix template." << endl
             << "void save_csv(const Tensor<type,2>&, const string&) method." << endl
             << "Cannot open matrix data file: " << filename << endl;

      throw invalid_argument(buffer.str());
    }

    file.precision(20);

    const Index data_rows = data.dimension(0);
    const Index data_columns = data.dimension(1);

    char separator_char = ';';

    for(Index i = 0; i < data_rows; i++)
    {
       for(Index j = 0; j < data_columns; j++)
       {
           file << data(i,j);

           if(j != data_columns-1)
           {
               file << separator_char;
           }
       }
       file << endl;
    }
    file.close();
}


Tensor<Index, 1> calculate_rank_greater(const Tensor<type, 1>& vector)
{
    const Index size = vector.size();

    Tensor<Index, 1> rank(size);
    iota(rank.data(), rank.data() + rank.size(), 0);

    sort(rank.data(),
         rank.data() + rank.size(),
         [&](Index i, Index j){return vector[i] > vector[j];});

    return rank;
}


Tensor<type, 2> box_plots_to_tensor(const Tensor<BoxPlot, 1>& box_plots)
{
   const Index columns_number = box_plots.dimension(0);

   Tensor<type, 2> summary(5, columns_number);

   for(Index i = 0; i < columns_number; i++)
   {
       const BoxPlot& box_plot = box_plots(i);
       summary(0, i) = box_plot.minimum;
       summary(1, i) = box_plot.first_quartile;
       summary(2, i) = box_plot.median;
       summary(3, i) = box_plot.third_quartile;
       summary(4, i) = box_plot.maximum;
   }

   Eigen::array<Index, 2> new_shape = {1, 5 * columns_number};
   Tensor<type, 2> reshaped_summary = summary.reshape(new_shape);

   return reshaped_summary;
}


Tensor<Index, 1> calculate_rank_less(const Tensor<type, 1>& vector)
{
    const Index size = vector.size();

    Tensor<Index, 1> rank(size);
    iota(rank.data(), rank.data() + rank.size(), 0);

    sort(rank.data(),
         rank.data() + rank.size(),
         [&](Index i, Index j){return vector[i] < vector[j];});

    return rank;
}


void scrub_missing_values(Tensor<type, 2>& matrix, const type& value)
{
    replace_if(matrix.data(), matrix.data()+matrix.size(), [](type x){return isnan(x);}, value);
}


Tensor<string, 1> sort_by_rank(const Tensor<string,1>&tokens, const Tensor<Index,1>&rank)
{
    const Index tokens_size = tokens.size();

    if(tokens_size != rank.size())
    {
        ostringstream buffer;

        buffer << "OpenNN Exception: Strings Class.\n"
               << "Tensor<string, 1> sort_by_rank(const Tensor<string,1>&tokens, const Tensor<Index,1>&rank) method.\n"
               << "Tokens and rank size must be the same.\n";

        throw invalid_argument(buffer.str());
    }

    Tensor<string,1> sorted_tokens(tokens_size);

    for(Index i = 0; i < tokens_size; i++)
    {
        sorted_tokens(i) = tokens(rank(i));
    }

    return sorted_tokens;
};



Tensor<Index, 1> sort_by_rank(const Tensor<Index,1>&tokens, const Tensor<Index,1>&rank)
{
    const Index tokens_size = tokens.size();

    if(tokens_size != rank.size())
    {
        ostringstream buffer;

        buffer << "OpenNN Exception: Strings Class.\n"
               << "Tensor<string, 1> sort_by_rank(const Tensor<string,1>&tokens, const Tensor<Index,1>&rank) method.\n"
               << "Tokens and rank size must be the same.\n";

        throw invalid_argument(buffer.str());
    }

    Tensor<Index,1> sorted_tokens(tokens_size);

    for(Index i = 0; i < tokens_size; i++)
    {
        sorted_tokens(i) = tokens(rank(i));
    }

    return sorted_tokens;
};



Index count_elements_less_than(const Tensor<Index,1>& vector, const Index& bound)
{
    Index count = 0;

    for(Index i = 0; i < vector.size(); i++)
    {
        if(vector(i) < bound)
            count++;
    }

    return count;
};



Tensor<Index, 1> get_indices_less_than(const Tensor<Index,1>& vector, const Index& bound)
{
    const Index indices_size = count_elements_less_than(vector, bound);

    Tensor<Index, 1> indices(indices_size);

    Index index = 0;

    for(Index i  = 0; i < vector.size(); i++)
    {
         if(vector(i) < bound)
         {
             indices(index) = i;
             index++;
         }
    }

    return indices;
};



Index count_elements_less_than(const Tensor<double,1>& vector, const double& bound)
{
    Index count = 0;

    for(Index i = 0; i < vector.size(); i++)
    {
        if(vector(i) < bound)
            count++;
    }

    return count;
};



Tensor<Index, 1> get_indices_less_than(const Tensor<double,1>& vector, const double& bound)
{
    const Index indices_size = count_elements_less_than(vector, bound);

    Tensor<Index, 1> indices(indices_size);

    Index index = 0;

    for(Index i  = 0; i < vector.size(); i++)
    {
         if(vector(i) < bound)
         {
             indices(index) = i;
             index++;
         }
    }

    return indices;
};


Index count_elements_greater_than(const Tensor<Index,1>& vector, const Index& bound)
{
    Index count = 0;

    for(Index i = 0; i < vector.size(); i++)
    {
        if(vector(i) > bound)
            count++;
    }

    return count;
};


Tensor<Index, 1> get_elements_greater_than(const Tensor<Index,1>& vector, const Index& bound)
{
    const Index indices_size = count_elements_greater_than(vector, bound);

    Tensor<Index, 1> indices(indices_size);

    Index index = 0;

    for(Index i  = 0; i < vector.size(); i++)
    {
         if(vector(i) > bound)
         {
             indices(index) = vector(i);
             index++;
         }
    }

    return indices;
};


Tensor<Index, 1> get_elements_greater_than(const Tensor<Tensor<Index, 1>,1>& vectors, const Index& bound)
{
    const Index vectors_number = vectors.size();

    Tensor<Index, 1> indices(0);

    for(Index i = 0; i < vectors_number; i++)
    {
        Tensor<Index, 1> indices_vector = get_elements_greater_than(vectors(i), bound);

        indices = join_vector_vector(indices, indices_vector);
    }

    return indices;
};


void delete_indices(Tensor<string,1>& vector, const Tensor<Index,1>& indices)
{
    const Index original_size = vector.size();

    const Index new_size = vector.size() - indices.size();

    Tensor<string,1> vector_copy(vector);

    vector.resize(new_size);

    Index index = 0;

    for(Index i = 0; i < original_size; i++)
    {
        if( !contains(indices, i) )
        {
            vector(index) = vector_copy(i);
            index++;
        }
    }
};



void delete_indices(Tensor<Index,1>& vector, const Tensor<Index,1>& indices)
{
    const Index original_size = vector.size();

    const Index new_size = vector.size() - indices.size();

    Tensor<Index,1> vector_copy(vector);

    vector.resize(new_size);

    Index index = 0;

    for(Index i = 0; i < original_size; i++)
    {
        if( !contains(indices, i) )
        {
            vector(index) = vector_copy(i);
            index++;
        }
    }
};



void delete_indices(Tensor<double,1>& vector, const Tensor<Index,1>& indices)
{
    const Index original_size = vector.size();

    const Index new_size = vector.size() - indices.size();

    Tensor<double,1> vector_copy(vector);

    vector.resize(new_size);

    Index index = 0;

    for(Index i = 0; i < original_size; i++)
    {
        if( !contains(indices, i) )
        {
            vector(index) = vector_copy(i);
            index++;
        }
    }
};



Tensor<string, 1> get_first(const Tensor<string,1>& vector, const Index& index)
{
    Tensor<string, 1> new_vector(index);

//    copy(new_vector.data(), new_vector.data() + index, vector.data());

    return new_vector;
};



Tensor<Index, 1> get_first(const Tensor<Index,1>& vector, const Index& index)
{
    Tensor<Index, 1> new_vector(index);

//    copy(new_vector.data(), new_vector.data() + index, vector.data());

    return new_vector;
};



/// Returns the number of elements which are equal or greater than a minimum given value
/// and equal or less than a maximum given value.
/// @param minimum Minimum value.
/// @param maximum Maximum value.

Index count_between(const Tensor<type,1>& vector,const type& minimum, const type& maximum)
{
    const Index size = vector.size();

    Index count = 0;

    for(Index i = 0; i < size; i++)
    {
        if(vector(i) >= minimum && vector(i) <= maximum) count++;
    }

    return count;
}


void set_row(Tensor<type,2>& matrix, Tensor<type,1>& new_row, const Index& row_index)
{
    const Index columns_number = new_row.size();

#ifdef __OPENNN_DEBUG__

    if(row_index >= rows_number)
    {
        ostringstream buffer;

        buffer << "OpenNN Exception: Matrix Template.\n"
               << "set_row(Tensor<type,2>& matrix, Tensor<type,1>& new_row, const Index& row_index) method.\n"
               << "Index must be less than number of rows.\n";

        throw logic_error(buffer.str());
    }
    if(columns_number != matrix.dimension(0))
    {
        ostringstream buffer;

        buffer << "OpenNN Exception: Matrix Template.\n"
               << "set_row(Tensor<type,2>& matrix, Tensor<type,1>& new_row, const Index& row_index) method.\n"
               << "New row must have same columns number than original matrix.\n";

        throw logic_error(buffer.str());
    }

#endif

    // Set new row

    for(Index i = 0; i < columns_number; i++)
    {
        matrix(row_index,i) = new_row(i);
    }
}


Tensor<type,2> filter_column_minimum_maximum(Tensor<type,2>& matrix,const Index& column_index, const type& minimum, const type& maximum)
{
#ifdef __OPENNN_DEBUG__

    if(column_index >= matrix.dimension(1))
    {
        ostringstream buffer;

        buffer << "OpenNN Exception: Matrix Template.\n"
               << "filter_column_minimum_maximum(Tensor<type,2>& matrix,const Index& column_index, const type& minimum, const type& maximum)\n"
               << "Column index must be less than number of columns.\n";

        throw logic_error(buffer.str());
    }

#endif
    const Tensor<type,1> column = matrix.chip(column_index,1);
    const Index new_rows_number = count_between(column, minimum, maximum);

    if(new_rows_number == 0)
    {
        return Tensor<type,2>();
    }

    const Index rows_number = matrix.dimension(0);
    const Index columns_number = matrix.dimension(1);

    bool check_conditions = false;

    Tensor<type,2> new_matrix(new_rows_number, columns_number);

    Index row_index = 0;
    Tensor<type,1> row(columns_number);

    for(Index i = 0; i < rows_number; i++)
    {
        if(matrix(i,column_index) >= minimum && matrix(i, column_index) <= maximum)
        {
            row = matrix.chip(i,0);

            set_row(new_matrix, row, row_index);

            row_index++;

            check_conditions = true;
        }
    }

    if(!check_conditions)
    {
        ostringstream buffer;

        buffer << "OpenNN Exception: TensorUtilities class.\n"
               << "Tensor<type,2> filter_column_minimum_maximum(Tensor<type,2>&,const Index&,const type&,const type&) method.\n"
               << "Invalid conditions\n";

        throw invalid_argument(buffer.str());
    }

    return new_matrix;
};


Tensor<type, 2> kronecker_product(Tensor<type, 1>& x, Tensor<type, 1>& y)
{
    // Transform Tensors into Dense matrix

    auto ml = Map<Matrix<type, Dynamic, Dynamic, RowMajor >>(x.data(), x.dimension(0), 1);

    auto mr = Map<Matrix<type, Dynamic, Dynamic, RowMajor>>(y.data(),y.dimension(0), 1);

    // Kronecker Product

    auto product = kroneckerProduct(ml, mr).eval();

    // Matrix into a Tensor

    TensorMap< Tensor<type, 2> > direct_matrix(product.data(), x.size(), y.size());

    return direct_matrix;
}


void kronecker_product_void(TensorMap<Tensor<type, 1>>& x, TensorMap<Tensor<type, 2>>& y)
{
    const Index n = x.dimension(0);
    auto x_matrix = Map< Matrix<type, Dynamic, Dynamic> >(x.data(), n, 1);

    auto product = Map< Matrix<type, Dynamic, Dynamic> >(y.data(), n*n, 1);

    product = kroneckerProduct(x_matrix, x_matrix).eval();
}


type l1_norm(const ThreadPoolDevice* thread_pool_device, const Tensor<type, 1>& vector)
{
    Tensor<type, 0> norm;

    norm.device(*thread_pool_device) = vector.abs().sum();

    return norm(0);
}


void l1_norm_gradient(const ThreadPoolDevice* thread_pool_device, const Tensor<type, 1>& vector, Tensor<type, 1>& gradient)
{
    gradient.device(*thread_pool_device) = vector.sign();
}


void l1_norm_hessian(const ThreadPoolDevice*, const Tensor<type, 1>&, Tensor<type, 2>& hessian)
{
    hessian.setZero();
}


/// Returns the l2 norm of a vector.

type l2_norm(const ThreadPoolDevice* thread_pool_device, const Tensor<type, 1>& vector)
{
    Tensor<type, 0> norm;

    norm.device(*thread_pool_device) = vector.square().sum().sqrt();

    if(isnan(norm(0)))
    {
        ostringstream buffer;

        buffer << "OpenNN Exception: l2 norm of vector is not a number."
               << endl;

        throw invalid_argument(buffer.str());
    }

    return norm(0);
}


void l2_norm_gradient(const ThreadPoolDevice* thread_pool_device, const Tensor<type, 1>& vector, Tensor<type, 1>& gradient)
{
    const type norm = l2_norm(thread_pool_device, vector);

    if(norm < type(NUMERIC_LIMITS_MIN))
    {
        gradient.setZero();

        return;
    }

    gradient.device(*thread_pool_device) = vector/norm;
}


void l2_norm_hessian(const ThreadPoolDevice* thread_pool_device, Tensor<type, 1>& vector, Tensor<type, 2>& hessian)
{
    const type norm = l2_norm(thread_pool_device, vector);

    if(norm < type(NUMERIC_LIMITS_MIN))
    {
        hessian.setZero();

        return;
    }

    hessian.device(*thread_pool_device) = kronecker_product(vector, vector)/(norm*norm*norm);
}


type l2_distance(const Tensor<type, 1>&x, const Tensor<type, 1>&y)
{
    if(x.size() != y.size())
    {
        ostringstream buffer;

        buffer << "OpenNN Exception: Tensor utilites.\n"
               << "type l2_distance(const Tensor<type, 1>&, const Tensor<type, 1>&)\n"
               << "x and y vector must  have the same dimensions.\n";

        throw invalid_argument(buffer.str());
    }

    Tensor<type, 0> distance;

    distance = (x-y).square().sum().sqrt();

    return distance(0);
}


type l2_distance(const Tensor<type, 2>&x, const Tensor<type, 2>&y)
{
    Tensor<type, 0> distance;

    distance = (x-y).square().sum().sqrt();

    return distance(0);
}


type l2_distance(const type& x, const type& y)
{
    const type distance = fabs(x - y);

    return distance;
}

Tensor<type, 1> l2_distance(const Tensor<type, 2>&x, const Tensor<type, 2>&y, const Index& size)
{
    Tensor<type, 1> distance(size);

    const Tensor<type, 2> difference = x - y;

    for(Index i = 0; i < difference.dimension(1); i++)
    {
        distance(i) = abs(difference(i));
    }

    return distance;
}


void sum_diagonal(Tensor<type, 2>& matrix, const type& value)
{
    const Index rows_number = matrix.dimension(0);

    #pragma omp parallel for

    for(Index i = 0; i < rows_number; i++)
        matrix(i,i) += value;
}


/// Uses Eigen to solve the system of equations by means of the Householder QR decomposition.

Tensor<type, 1> perform_Householder_QR_decomposition(const Tensor<type, 2>& A, const Tensor<type, 1>& b)
{
    const Index n = A.dimension(0);

    Tensor<type, 1> x(n);

    const Map<Matrix<type, Dynamic, Dynamic>> A_eigen((type*)A.data(), n, n);
    const Map<Matrix<type, Dynamic, 1>> b_eigen((type*)b.data(), n, 1);
    Map<Matrix<type, Dynamic, 1>> x_eigen((type*)x.data(), n);

    x_eigen = A_eigen.colPivHouseholderQr().solve(b_eigen);

    return x;
}


void fill_submatrix(const Tensor<type, 2>& matrix,
                    const Tensor<Index, 1>& rows_indices,
                    const Tensor<Index, 1>& columns_indices,
                    type* submatrix_pointer)
{
    const Index rows_number = rows_indices.size();
    const Index columns_number = columns_indices.size();

    const type* matrix_pointer = matrix.data();

    #pragma omp parallel for

    for(Index j = 0; j < columns_number; j++)
    {
        const type* matrix_column_pointer = matrix_pointer + matrix.dimension(0)*columns_indices[j];
        type* submatrix_column_pointer = submatrix_pointer + rows_number*j;

        const type* value_pointer = nullptr;
        const Index* rows_indices_pointer = rows_indices.data();
        for(Index i = 0; i < rows_number; i++)
        {
            value_pointer = matrix_column_pointer + *rows_indices_pointer;
            rows_indices_pointer++;
            *submatrix_column_pointer = *value_pointer;
            submatrix_column_pointer++;
        }
    }
}

//void fill_submatrix(const Tensor<type, 2>& matrix,
//    const Tensor<Index, 1>& rows_indices,
//    const Tensor<Index, 1>& columns_indices,
//    Tensor<type, 2>& submatrix)
//{
//    Map<const Matrix<type, Eigen::Dynamic, Eigen::Dynamic>> matrix_map(matrix.data(), matrix.dimension(0), matrix.dimension(1));

//    Map<Matrix<type, Eigen::Dynamic, Eigen::Dynamic>> submatrix_map(submatrix.data(), submatrix.dimension(0), submatrix.dimension(1));

//    submatrix_map = matrix_map(rows_indices, columns_indices);
//}


Index count_NAN(const Tensor<type, 1>& x)
{
    Index NAN_number = 0;

    for(Index i = 0; i < x.size(); i++)
    {
        if(isnan(x(i))) NAN_number++;
    }

    return NAN_number;
}


Index count_NAN(const Tensor<type, 2>& x)
{
    const Index rows_number = x.dimension(0);
    const Index columns_number = x.dimension(1);

    Index count = 0;

    #pragma omp parallel for reduction(+: count)

    for(Index row_index = 0; row_index < rows_number; row_index++)
    {
        for(Index column_index = 0; column_index < columns_number; column_index++)
        {
            if(isnan(x(row_index, column_index))) count++;
        }
    }

    return count;
}


bool has_NAN(const Tensor<type, 1>& x)
{
    for(Index i = 0; i < x.size(); i++)
    {
        if(isnan(x(i))) return true;
    }

    return false;
}


bool has_NAN(Tensor<type, 2>& x)
{
    for(Index i = 0; i < x.size(); i++)
    {
        if(isnan(x(i))) return true;
    }

    return false;
}

Index count_empty_values(const Tensor<string, 1>& vector)
{
    const Index words_number = vector.size();

    Index count = 0;

    string empty_string;

    for( Index i = 0; i < words_number; i++)
    {
        string str = vector(i);
        trim(str);
        if(str == empty_string) count++;
    }

    return count;
}


void check_size(const Tensor<type, 1>& vector, const Index& size, const string& log)
{
    if(vector.size() != size)
    {
        ostringstream buffer;

        buffer << "OpenNN Exception: " << log <<  endl
               << "Size of vector is " << vector.size() << ", but must be " << size << "." << endl;

        throw invalid_argument(buffer.str());
    }
}


void check_dimensions(const Tensor<type, 2>& matrix, const Index& rows_number, const Index& columns_number, const string& log)
{
    if(matrix.dimension(0) != rows_number)
    {
        ostringstream buffer;

        buffer << "OpenNN Exception: " << log <<  endl
               << "Number of rows in matrix is " << matrix.dimension(0) << ", but must be " << rows_number << "." <<  endl;

        throw invalid_argument(buffer.str());
    }

    if(matrix.dimension(1) != columns_number)
    {
        ostringstream buffer;

        buffer << "OpenNN Exception: " << log <<  endl
               << "Number of columns in matrix is " << matrix.dimension(0) << ", but must be " << columns_number << "." <<  endl;

        throw invalid_argument(buffer.str());
    }
}


void check_columns_number(const Tensor<type, 2>& matrix, const Index& columns_number, const string& log)
{
    if(matrix.dimension(1) != columns_number)
    {
        ostringstream buffer;

        buffer << "OpenNN Exception: " << log <<  endl
               << "Number of columns in matrix is " << matrix.dimension(0) << ", but must be " << columns_number << "." <<  endl;

        throw invalid_argument(buffer.str());
    }
}


void check_rows_number(const Tensor<type, 2>& matrix, const Index& rows_number, const string& log)
{
    if(matrix.dimension(1) != rows_number)
    {
        ostringstream buffer;

        buffer << "OpenNN Exception: " << log <<  endl
               << "Number of columns in matrix is " << matrix.dimension(0) << ", but must be " << rows_number << "." <<  endl;

        throw invalid_argument(buffer.str());
    }
}


Tensor<Index, 1> join_vector_vector(const Tensor<Index, 1>& x, const Tensor<Index, 1>& y)
{
    const Index size = x.size() + y.size();

    Tensor<Index, 1> data(size);

    copy(x.data(), x.data() + x.size(), data.data());
    copy(y.data(), y.data() + y.size(), data.data() + x.size());

    return data;
}


Tensor<type, 2> assemble_vector_vector(const Tensor<type, 1>& x, const Tensor<type, 1>& y)
{
    const Index rows_number = x.size();
    const Index columns_number = 2;

    Tensor<type, 2> data(rows_number, columns_number);

    for(Index i = 0; i < rows_number; i++)
    {
        data(i, 0) = x(i);
        data(i, 1) = y(i);
    }

    return data;
}


Tensor<type, 2> assemble_vector_matrix(const Tensor<type, 1>& x, const Tensor<type, 2>& y)
{
    const Index rows_number = x.size();
    const Index columns_number = 1 + y.dimension(1);

    Tensor<type, 2> data(rows_number, columns_number);

    for(Index i = 0; i < rows_number; i++)
    {
        data(i, 0) = x(i);

        for(Index j = 0; j < y.dimension(1); j++)
        {
            data(i, 1+j) = y(i,j);
        }
    }

    return data;
}


Tensor<type, 2> assemble_matrix_vector(const Tensor<type, 2>& x, const Tensor<type, 1>& y)
{
    const Index rows_number = y.size();
    const Index columns_number = x.dimension(1) + 1;

    Tensor<type, 2> data(rows_number, columns_number);

    for(Index i = 0; i < rows_number; i++)
    {
        for(Index j = 0; j < x.dimension(1); j++)
        {
            data(i, j) = x(i,j);
        }

        data(i, columns_number-1) = y(i);
    }

    return data;
}


Tensor<type, 2> assemble_matrix_matrix(const Tensor<type, 2>& x, const Tensor<type, 2>& y)
{
    const Index rows_number = x.dimension(0);
    const Index columns_number = x.dimension(1) + y.dimension(1);

    Tensor<type, 2> data(rows_number, columns_number);

    for(Index i = 0; i < rows_number; i++)
    {
        for(Index j = 0; j < x.dimension(1); j++)
        {
            data(i,j) = x(i,j);
        }

        for(Index j = 0; j < y.dimension(1); j++)
        {
            data(i, x.dimension(1) + j) = y(i,j);
        }
    }

    return data;
}


Tensor<string, 1> assemble_text_vector_vector(const Tensor<string, 1>& x, const Tensor<string, 1>& y)
{
    const Index x_size = x.size();
    const Index y_size = y.size();

    Tensor<string,1> data(x_size + y_size);

    for(Index i = 0; i < x_size; i++)
    {
        data(i) = x(i);
    }

    for(Index i = 0; i < y_size; i++)
    {
        data(i + x_size) = y(i);
    }

    return data;
}


string tensor_string_to_text(const Tensor<string,1>&x, string& separator)
{
    const Index size = x.size();

    if(x.size() == 0)
    {
       ostringstream buffer;

       buffer << "OpenNN Exception: Tensor utilites.\n"
              << "Mstring tensor_string_to_text(Tensor<string,1>&x,string& separator).\n"
              << "Input vector must have dimension greater than 0.\n";

       throw invalid_argument(buffer.str());
    }

    string line = x(0);

    for(Index i = 1; i < size; i++)
    {
        line = line + separator+ x(i);
    }

    return line;

}


Tensor<type, 2> delete_row(const Tensor<type, 2>& tensor, const Index& row_index)
{
    const Index rows_number = tensor.dimension(0);
    const Index columns_number = tensor.dimension(1);
   #ifdef OPENNN_DEBUG

   if(row_index > rows_number)
   {
      ostringstream buffer;

      buffer << "OpenNN Exception: Matrix Template.\n"
             << "Matrix<T> delete_row(const size_t&) const.\n"
             << "Index of row must be less than number of rows.\n"
             << "row index: " << row_index << "rows_number" << rows_number << "\n";

      throw invalid_argument(buffer.str());
   }
   else if(rows_number < 2)
   {
      ostringstream buffer;

      buffer << "OpenNN Exception: Matrix Template.\n"
             << "Matrix<T> delete_row(const size_t&) const.\n"
             << "Number of rows must be equal or greater than two.\n";

      throw invalid_argument(buffer.str());
   }

   #endif

   Tensor<type, 2> new_matrix(rows_number-1, columns_number);

   for(Index i = 0; i < row_index; i++)
   {
      for(Index j = 0; j < columns_number; j++)
      {
        new_matrix(i,j) = tensor(i,j);
      }
   }

   for(Index i = row_index+1; i < rows_number; i++)
   {
      for(Index j = 0; j < columns_number; j++)
      {
         new_matrix(i-1,j) = tensor(i,j);
      }
   }

   return new_matrix;
}


/// Returns true if any value is less or equal than a given value, and false otherwise.

bool is_less_than(const Tensor<type, 1>& column, const type& value)
{
    const Tensor<bool, 1> if_sentence = (column <= column.constant(value));

    Tensor<bool, 1> sentence(column.size());
    sentence.setConstant(true);

    Tensor<bool, 1> else_sentence(column.size());
    else_sentence.setConstant(false);

    const Tensor<bool, 0> is_less = (if_sentence.select(sentence, else_sentence)).any();

    return is_less(0);
}


bool contains(const Tensor<size_t,1>& vector, const size_t& value)
{
    Tensor<size_t, 1> copy(vector);

    const size_t* it = find(copy.data(), copy.data()+copy.size(), value);

    return it != (copy.data()+copy.size());
}


bool contains(const Tensor<type,1>& vector, const type& value)
{
    Tensor<type, 1> copy(vector);

    const type* it = find(copy.data(), copy.data()+copy.size(), value);

    return it != (copy.data()+copy.size());
}


bool contains(const Tensor<Index,1>& vector, const Index& value)
{
    Tensor<Index, 1> copy(vector);

    const Index* it = find(copy.data(), copy.data()+copy.size(), value);

    return it != (copy.data()+copy.size());
}


bool contains(const Tensor<string,1>& vector, const string& value)
{
    Tensor<string, 1> copy(vector);

    const string* it = find(copy.data(), copy.data()+copy.size(), value);

    return it != (copy.data()+copy.size());
};


void push_back_index(Tensor<Index, 1>& old_vector, const Index& new_element)
{
    const Index old_size = old_vector.size();

    const Index new_size = old_size+1;

    Tensor<Index, 1> new_vector(new_size);

    for(Index i = 0; i < old_size; i++) new_vector(i) = old_vector(i);

    new_vector(new_size-1) = new_element;

    old_vector = new_vector;
}


void push_back_string(Tensor<string, 1>& old_vector, const string& new_string)
{
    const Index old_size = old_vector.size();

    const Index new_size = old_size+1;

    Tensor<string, 1> new_vector(new_size);

    for(Index i = 0; i < old_size; i++) new_vector(i) = old_vector(i);

    new_vector(new_size-1) = new_string;

    old_vector = new_vector;
}


void push_back_type(Tensor<type, 1>& vector, const type& new_value)
{
    const Index old_size = vector.size();

    const Index new_size = old_size+1;

    Tensor<type, 1> new_vector(new_size);

    for(Index i = 0; i < old_size; i++) new_vector(i) = vector(i);

    new_vector(new_size-1) = new_value;

    vector = new_vector;
}


Tensor<string, 1> to_string_tensor(const Tensor<type,1>& x)
{
    Tensor<string, 1> vector(x.size());

    for(Index i = 0; i < x.size(); i++)
    {
        vector(i) = to_string(x(i));
    }

    return vector;
};


void print_tensor(const float* vector, const int dimensions[])
{
    cout<<"Tensor"<<endl;

    const int rows_number = dimensions[0];
    const int cols_number = dimensions[1];
    const int channels = dimensions[2];
    const int batch = dimensions[3];

    for(int l = 0; l<batch; l++)
    {
        for(int k = 0; k<channels; k++)
        {
            for(int i = 0; i<rows_number; i++)
            {
                for(int j = 0; j<cols_number; j++)
                {
                    if(i + rows_number*j + k*rows_number*cols_number + l*channels*rows_number*cols_number % rows_number*cols_number*channels == 0)

                        cout<< "<--Batch-->"<<endl;

                    if(i + rows_number*j + k*rows_number*cols_number % rows_number*cols_number == 0)

                        cout<< "*--Channel--*"<<endl;

                   cout<<*(vector + i + j*rows_number + k*rows_number*cols_number+ l*channels*rows_number*cols_number)<< " ";
                }

               cout<<" "<<endl;
            }
        }
    }
}

void swap_rows(Tensor<type, 2>& data_matrix, Index row1, Index row2)
{
    Tensor<type, 1> temp = data_matrix.chip(row1, 0);
    data_matrix.chip(row1, 0) = data_matrix.chip(row2, 0);
    data_matrix.chip(row2, 0) = temp;
}


void quick_sort(Tensor<type, 2>& data, Index start_index, Index end_index, Index target_column)
{
    if (start_index >= end_index)
        return;

    Index partition_index = partition(data, start_index, end_index, target_column);

    quick_sort(data, start_index, partition_index - 1, target_column);
    quick_sort(data, partition_index + 1, end_index, target_column);
}


void quicksort_by_column(Tensor<type, 2>& data, Index target_column)
{
    Tensor<type, 2>* data_matrix_ptr = &data;
    Index number_of_rows = data_matrix_ptr->dimension(0);

    quick_sort(*data_matrix_ptr, 0, number_of_rows - 1, target_column);
}

Tensor<type, 1> compute_elementwise_difference(Tensor<type, 1>& data)
{
    if (data.size() <= 2) {
        return Tensor<type, 1>();
    }

    Tensor<type, 1> difference_data(data.size());
    difference_data(0) = 0;

    if (data.size() <= 1) return Tensor<type, 1>();

    #pragma omp parallel for
    for (int i = 1; i < data.size(); i++) difference_data(i) = data(i) - data(i - 1);

    return difference_data;
}

Tensor<type, 1> compute_mode(Tensor<type, 1>& data)
{
    Tensor<type, 1> mode_and_frequency(2);

    std::map<type, type> frequency_map;

    for (int i = 0; i < data.size(); i++)
    {
        type value = data(i);
        frequency_map[value]++;
    }

    cout << "frequency_map: " << endl;
    
    for(auto it = frequency_map.cbegin(); it != frequency_map.cend(); ++it)
    {
        cout << "Key: " << it->first << ", Value: " << it->second << "\n";
    }

    type mode = -1;
    type max_frequency = 0;

    for (const auto& entry : frequency_map)
    {
        if (entry.second > max_frequency)
        {
            max_frequency = entry.second;
            mode = entry.first;
        }
    }

    if(mode == -1) Tensor<type, 1>();

    mode_and_frequency(0) = mode;
    mode_and_frequency(1) = max_frequency;

    cout << "mode_and_frequency: " << mode_and_frequency << endl;

    return mode_and_frequency;
}


Tensor<type, 1> fill_gaps_by_value(Tensor<type, 1>& data, Tensor<type, 1>& difference_data, type value)
{
    std::vector<type> result;

    for(Index i = 1; i < difference_data.size(); i++)
    {
        if(difference_data(i) != value)
        {
            type previous_time = data(i-1) + value;
            do
            {
                result.push_back(previous_time);

                previous_time += value;
            } while (previous_time < data(i));                
        }
    }

    TensorMap<Tensor<type, 1>> filled_data(result.data(), result.size());

    return filled_data;
}


Index partition(Tensor<type, 2>& data_matrix, Index start_index, Index end_index, Index target_column)
{
    Tensor<type, 1> pivot_row = data_matrix.chip(start_index, 0);
    type pivot_value = pivot_row(target_column);
    Index smaller_elements_count = 0;

    for (Index current_index = start_index + 1; current_index <= end_index; current_index++)
    {
        if (data_matrix(current_index, target_column) <= pivot_value)
        {
            smaller_elements_count++;
        }
    }

    Index pivot_position = start_index + smaller_elements_count;
    swap_rows(data_matrix, pivot_position, start_index);

    Index left_index = start_index, right_index = end_index;

    while (left_index < pivot_position && right_index > pivot_position)
    {
        while (data_matrix(left_index, target_column) <= pivot_value)
        {
            left_index++;
        }

        while (data_matrix(right_index, target_column) > pivot_value)
        {
            right_index--;
        }

        if (left_index < pivot_position && right_index > pivot_position)
        {
            swap_rows(data_matrix, left_index++, right_index--);
        }
    }

    return pivot_position;
}


Tensor<Index, 1> intersection(const Tensor<Index, 1>& tensor1, const Tensor<Index, 1>& tensor2)
{
    Index intersection_index_number = 0;

    for (Index i = 0; i < tensor1.size(); i++)
    {
        for (Index j = 0; j < tensor2.size(); j++)
        {
            if (tensor1(i) == tensor2(j)) {
                intersection_index_number++;
            }
        }
    }

    if (intersection_index_number == 0)
    {
        return Tensor<Index, 1>(0);
    }

    Tensor<Index, 1> intersection(intersection_index_number);
    Index count = 0;

    for (Index i = 0; i < tensor1.size(); i++)
    {
        for (Index j = 0; j < tensor2.size(); j++)
        {
            if (tensor1(i) == tensor2(j))
            {
                intersection(count) = tensor2(j);
                count++;
            }
        }
    }

    return intersection;
}

}


// OpenNN: Open Neural Networks Library.
// Copyright(C) 2005-2023 Artificial Intelligence Techniques, SL.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
