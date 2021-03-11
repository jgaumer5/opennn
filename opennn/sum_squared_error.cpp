//   OpenNN: Open Neural Networks Library
//   www.opennn.net
//
//   S U M   S Q U A R E D   E R R O R   C L A S S
//
//   Artificial Intelligence Techniques SL
//   artelnics@artelnics.com

#include "sum_squared_error.h"

namespace OpenNN
{

/// Default constructor.
/// It creates a sum squared error term not associated to any neural network and not measured on any data set.
/// It also initializes all the rest of class members to their default values.

SumSquaredError::SumSquaredError() : LossIndex()
{
}


/// Neural network and data set constructor.
/// It creates a sum squared error associated to a neural network and measured on a data set.
/// It also initializes all the rest of class members to their default values.
/// @param new_neural_network_pointer Pointer to a neural network object.
/// @param new_data_set_pointer Pointer to a data set object.

SumSquaredError::SumSquaredError(NeuralNetwork* new_neural_network_pointer, DataSet* new_data_set_pointer)
    : LossIndex(new_neural_network_pointer, new_data_set_pointer)
{
}


/// Destructor.

SumSquaredError::~SumSquaredError()
{
}


void SumSquaredError::calculate_error(const DataSetBatch& batch,
                     const NeuralNetworkForwardPropagation& forward_propagation,
                     BackPropagation& back_propagation) const
{
    Tensor<type, 0> sum_squared_error;

    sum_squared_error.device(*thread_pool_device) = back_propagation.errors.contract(back_propagation.errors, SSE);

    back_propagation.error = sum_squared_error(0);
}


void SumSquaredError::calculate_squared_errors(const DataSetBatch& batch,
                                            const NeuralNetworkForwardPropagation& forward_propagation,
                                            LossIndexBackPropagationLM& second_order_loss) const
{
    const Index trainable_layers_number = neural_network_pointer->get_trainable_layers_number();

    const Layer* output_layer_pointer = neural_network_pointer->get_output_layer_pointer();

    LayerForwardPropagation* output_layer_forward_propagation = forward_propagation.layers(trainable_layers_number-1);

    const Eigen::array<int, 1> rows_sum = {Eigen::array<int, 1>({1})};

    const Tensor<type, 2>& targets = batch.targets_2d;

    switch(output_layer_pointer->get_type())
    {
    case Layer::Perceptron:
    {
        PerceptronLayerForwardPropagation* perceptron_layer_forward_propagation
        = static_cast<PerceptronLayerForwardPropagation*>(output_layer_forward_propagation);

        const Tensor<type, 2>& outputs = perceptron_layer_forward_propagation->activations;

        second_order_loss.squared_errors.device(*thread_pool_device) = ((outputs - targets).square().sum(rows_sum)).sqrt();
    }
        break;

    case Layer::Probabilistic:
    {
        ProbabilisticLayerForwardPropagation* probabilistic_layer_forward_propagation
        = static_cast<ProbabilisticLayerForwardPropagation*>(output_layer_forward_propagation);

        const Tensor<type, 2>& outputs = probabilistic_layer_forward_propagation->activations;

        second_order_loss.squared_errors.device(*thread_pool_device) = ((outputs - targets).square().sum(rows_sum)).sqrt();
    }
        break;

    case Layer::Recurrent:
    {
        RecurrentLayerForwardPropagation* recurrent_layer_forward_propagation
        = static_cast<RecurrentLayerForwardPropagation*>(output_layer_forward_propagation);

        const Tensor<type, 2>& outputs = recurrent_layer_forward_propagation->activations;

        second_order_loss.squared_errors.device(*thread_pool_device) = ((outputs - targets).square().sum(rows_sum)).sqrt();
    }
        break;

    case Layer::LongShortTermMemory:
    {
        LongShortTermMemoryLayerForwardPropagation* long_short_term_memory_layer_forward_propagation
        = static_cast<LongShortTermMemoryLayerForwardPropagation*>(output_layer_forward_propagation);

        const Tensor<type, 2>& outputs = long_short_term_memory_layer_forward_propagation->activations;

        second_order_loss.squared_errors.device(*thread_pool_device) = ((outputs - targets).square().sum(rows_sum)).sqrt();
    }
        break;

    default: break;
    }

    Tensor<type, 0> error;
    error.device(*thread_pool_device) = second_order_loss.squared_errors.contract(second_order_loss.squared_errors, AT_B);

    second_order_loss.error = error();
}


void SumSquaredError::calculate_output_delta(const DataSetBatch& batch,
                                             NeuralNetworkForwardPropagation& forward_propagation,
                                             BackPropagation& back_propagation) const
{
     #ifdef __OPENNN_DEBUG__

     check();

     #endif

     const Index trainable_layers_number = neural_network_pointer->get_trainable_layers_number();

     Layer* output_layer_pointer = neural_network_pointer->get_output_layer_pointer();

     LayerBackPropagation* output_layer_back_propagation = back_propagation.neural_network.layers(trainable_layers_number-1);

     const type coefficient = static_cast<type>(2.0);

     switch(output_layer_pointer->get_type())
     {
     case Layer::Perceptron:
     {
         PerceptronLayerBackPropagation* perceptron_layer_back_propagation
         = static_cast<PerceptronLayerBackPropagation*>(output_layer_back_propagation);

         perceptron_layer_back_propagation->delta.device(*thread_pool_device) = coefficient*back_propagation.errors;
     }
         break;

     case Layer::Probabilistic:
     {
         ProbabilisticLayerBackPropagation* probabilistic_layer_back_propagation
         = static_cast<ProbabilisticLayerBackPropagation*>(output_layer_back_propagation);

         probabilistic_layer_back_propagation->delta.device(*thread_pool_device) = coefficient*back_propagation.errors;
     }
         break;

     case Layer::Recurrent:
     {
         RecurrentLayerBackPropagation* recurrent_layer_back_propagation
         = static_cast<RecurrentLayerBackPropagation*>(output_layer_back_propagation);

         recurrent_layer_back_propagation->delta.device(*thread_pool_device) = coefficient*back_propagation.errors;
     }
         break;

     case Layer::LongShortTermMemory:
     {
         LongShortTermMemoryLayerBackPropagation* long_short_term_memory_layer_back_propagation
         = static_cast<LongShortTermMemoryLayerBackPropagation*>(output_layer_back_propagation);

         long_short_term_memory_layer_back_propagation->delta.device(*thread_pool_device) = coefficient*back_propagation.errors;
     }
         break;

     default: break;
     }
}


void SumSquaredError::calculate_Jacobian_gradient(const DataSetBatch& ,
                                    LossIndexBackPropagationLM& second_order_loss) const
{
#ifdef __OPENNN_DEBUG__

    check();

#endif

    const type coefficient = (static_cast<type>(2.0));

    second_order_loss.gradient.device(*thread_pool_device) = second_order_loss.squared_errors_Jacobian.contract(second_order_loss.squared_errors, AT_B);

    second_order_loss.gradient.device(*thread_pool_device) = coefficient*second_order_loss.gradient;
}


// Hessian method

void SumSquaredError::calculate_hessian_approximation(const DataSetBatch&, LossIndexBackPropagationLM& second_order_loss) const
{
     #ifdef __OPENNN_DEBUG__

     check();

     #endif

     const type coefficient = (static_cast<type>(2.0));

     second_order_loss.hessian.device(*thread_pool_device) = second_order_loss.squared_errors_Jacobian.contract(second_order_loss.squared_errors_Jacobian, AT_B);

     second_order_loss.hessian.device(*thread_pool_device) = coefficient*second_order_loss.hessian;
}


/// Returns a string with the name of the sum squared error loss type, "SUM_SQUARED_ERROR".

string SumSquaredError::get_error_type() const
{
    return "SUM_SQUARED_ERROR";
}


/// Returns a string with the name of the sum squared error loss type in text format.

string SumSquaredError::get_error_type_text() const
{
    return "Sum squared error";
}


/// Serializes the cross entropy error object into a XML document of the TinyXML library without keep the DOM tree in memory.
/// See the OpenNN manual for more information about the format of this document

void SumSquaredError::write_XML(tinyxml2::XMLPrinter& file_stream) const
{
    // Error type

    file_stream.OpenElement("SumSquaredError");

    file_stream.CloseElement();
}


/// Loads a sum squared error object from a XML document.
/// @param document TinyXML document containing the members of the object.

void SumSquaredError::from_XML(const tinyxml2::XMLDocument& document)
{
    const tinyxml2::XMLElement* root_element = document.FirstChildElement("SumSquaredError");

    if(!root_element)
    {
        ostringstream buffer;

        buffer << "OpenNN Exception: SumSquaredError class.\n"
               << "void from_XML(const tinyxml2::XMLDocument&) method.\n"
               << "Sum squared element is nullptr.\n";

        throw logic_error(buffer.str());
    }
}

}

// OpenNN: Open Neural Networks Library.
// Copyright(C) 2005-2021 Artificial Intelligence Techniques, SL.
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
