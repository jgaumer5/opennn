//   OpenNN: Open Neural Networks Library
//   www.opennn.net
//
//   C O N V O L U T I O N A L   L A Y E R   C L A S S
//
//   Artificial Intelligence Techniques SL
//   artelnics@artelnics.com

#include "convolutional_layer.h"

namespace opennn
{

/// Default constructor.
/// It creates an empty ConvolutionalLayer object.

ConvolutionalLayer::ConvolutionalLayer() : Layer()
{
    layer_type = Layer::Type::Convolutional;
}


ConvolutionalLayer::ConvolutionalLayer(const Index& new_inputs_number,
                                       const Index& new_outputs_number,
                                       const ConvolutionalLayer::ActivationFunction& new_activation_function) : Layer()
{
    layer_type = Layer::Type::Convolutional;
}


/// Inputs' dimensions modifier constructor.
/// After setting new dimensions for the inputs, it creates and initializes a ConvolutionalLayer object
/// with a number of kernels of a given size.
/// The initialization values are random values from a normal distribution.
/// @param new_inputs_dimensions A vector containing the new inputs' dimensions.
/// @param kernels_dimensions A vector containing the kernel rows, columns channels and number.

ConvolutionalLayer::ConvolutionalLayer(const Tensor<Index, 1>& new_inputs_dimensions,
                                       const Tensor<Index, 1>& new_kernels_dimensions) : Layer()
{
    layer_type = Layer::Type::Convolutional;

    set(new_inputs_dimensions, new_kernels_dimensions);
}


/// Returns a boolean, true if convolutional layer is empty and false otherwise.

bool ConvolutionalLayer::is_empty() const
{
    if(biases.size() == 0 && synaptic_weights.size() == 0)
    {
        return true;
    }

    return false;
}

/// Returns the layer's biases.

const Tensor<type, 1>& ConvolutionalLayer::get_biases() const
{
    return biases;
}


/// Returns the layer's synaptic weights.

const Tensor<type, 4>& ConvolutionalLayer::get_synaptic_weights() const
{
    return synaptic_weights;
}


/// Inserts padding to the input tensor.
/// @param input Tensor containing the inputs.
/// @param padded_output input tensor padded.

void ConvolutionalLayer::insert_padding(const Tensor<type, 4>& inputs, Tensor<type, 4>& padded_output)
{
    switch(convolution_type)
    {
    case ConvolutionType::Valid: padded_output = inputs; return;

    case ConvolutionType::Same:
    {
        const Index input_rows_number = inputs.dimension(2);
        const Index input_cols_number = inputs.dimension(3);

        const Index kernel_rows_number = get_kernels_rows_number();
        const Index kernel_cols_number = get_kernels_columns_number();

        Eigen::array<pair<int, int>, 4> paddings;

        const int pad_rows = int(0.5 *(input_rows_number*(row_stride - 1) - row_stride + kernel_rows_number));
        const int pad_cols = int(0.5 *(input_cols_number*(column_stride - 1) - column_stride + kernel_cols_number));

        paddings[0] = make_pair(pad_rows, pad_rows);
        paddings[1] = make_pair(pad_cols, pad_cols);
        paddings[2] = make_pair(0, 0);
        paddings[3] = make_pair(0, 0);

        padded_output = inputs.pad(paddings);
        return;
    }
    }
}


/// Calculate convolutions

void ConvolutionalLayer::calculate_convolutions(const Tensor<type, 4>& inputs,
                                                type* convolutions_data) const
{
    const Index images_number = inputs.dimension(0);
    const Index inputs_channels_number = inputs.dimension(1);
    const Index inputs_rows_number = inputs.dimension(2);
    const Index inputs_columns_number = inputs.dimension(3);

    const Index kernels_number = synaptic_weights.dimension(0);
    const Index kernels_channels_number = synaptic_weights.dimension(1);
    const Index kernels_rows_number = synaptic_weights.dimension(2);
    const Index kernels_columns_number = synaptic_weights.dimension(3);

#ifdef OPENNN_DEBUG
/*
    const Index output_channel_number = combinations.dimension(2);
    const Index output_images_number = combinations.dimension(3);

    if(output_channel_number != kernels_number)
    {
        ostringstream buffer;
        buffer << "OpenNN Exception: ConvolutionalLayer class.\n"
               << "ConvolutionalLayer::calculate_convolutions.\n"
               << "output_channel_number" <<output_channel_number <<"must me equal to" << kernels_number<<".\n";

        throw invalid_argument(buffer.str());
    }

    if(output_images_number != images_number)
    {
        ostringstream buffer;
        buffer << "OpenNN Exception: ConvolutionalLayer class.\n"
               << "ConvolutionalLayer::calculate_convolutions.\n"
               << "output_images_number" <<output_images_number <<"must me equal to" << images_number<<".\n";

        throw invalid_argument(buffer.str());
    }
*/
#endif

    const Index image_size = inputs_channels_number*inputs_rows_number*inputs_columns_number;
    const Index kernel_size = kernels_channels_number*kernels_rows_number*kernels_columns_number;

    const Index outputs_rows_number = get_outputs_rows_number();
    const Index outputs_columns_number = get_outputs_columns_number();

    type* inputs_pointer = const_cast<type*>(inputs.data());
    type* synaptic_weights_pointer = const_cast<type*>(synaptic_weights.data());

    Tensor<type, 3> convolved_image(kernels_number, outputs_rows_number, outputs_columns_number);

    #pragma omp parallel for

    for(int i = 0; i < images_number ;i++)
    {
        const TensorMap<Tensor<type, 3>> image(inputs_pointer + i*image_size,
                                               inputs_channels_number,
                                               inputs_rows_number,
                                               inputs_columns_number);

        for(int j = 0; j < kernels_number; j++)
        {
            const TensorMap<Tensor<type, 3>> kernel(synaptic_weights_pointer + j*kernel_size,
                                                    kernels_channels_number,
                                                    kernels_rows_number,
                                                    kernels_columns_number);

            convolved_image = image.convolve(kernel, convolution_dimensions) + biases(j);

            memcpy(convolutions_data + j*outputs_rows_number*outputs_columns_number + i*outputs_rows_number*outputs_columns_number*kernels_number,
                   convolved_image.data(), static_cast<size_t>(outputs_rows_number*outputs_columns_number)*sizeof(type));
        }
    }
}


void ConvolutionalLayer::calculate_convolutions(const Tensor<type, 4>& inputs,
                                                const Tensor<type, 2>& potential_biases,
                                                const Tensor<type, 4>& potential_synaptic_weights,
                                                Tensor<type, 4>& convolutions) const // old version
{
    const Index images_number = inputs.dimension(0);
    const Index inputs_channels_number = inputs.dimension(1);
    const Index inputs_rows_number = inputs.dimension(2);
    const Index inputs_columns_number = inputs.dimension(3);

    const Index kernels_number = potential_synaptic_weights.dimension(0);
    const Index kernels_channels_number = potential_synaptic_weights.dimension(1);
    const Index kernels_rows_number = potential_synaptic_weights.dimension(2);
    const Index kernels_columns_number = potential_synaptic_weights.dimension(3);

    const Index next_image = inputs_rows_number*inputs_columns_number*inputs_channels_number;
    const Index next_kernel = kernels_rows_number*kernels_columns_number*kernels_channels_number;

    const Index output_size_rows_columns = ((inputs_rows_number-kernels_rows_number)+1)*((inputs_columns_number-kernels_columns_number)+1);

    const Eigen::array<ptrdiff_t, 3> dims = {0, 1, 2};

    Tensor<type, 4> inputs_pointer = inputs;
    Tensor<type, 4> synaptic_weights_pointer = potential_synaptic_weights; // ??

#pragma omp parallel for
    for(int i = 0; i < images_number ;i++)
    {
        const TensorMap<Tensor<type, 3>> single_image(inputs_pointer.data()+i*next_image,
                                                      inputs_channels_number,
                                                      inputs_rows_number,
                                                      inputs_columns_number);

        for(int j = 0; j<kernels_number; j++)
        {
            const TensorMap<Tensor<type, 3>> single_kernel(synaptic_weights_pointer.data()+j*next_kernel,
                                                           kernels_channels_number,
                                                           kernels_rows_number,
                                                           kernels_columns_number);

            Tensor<type, 3> tmp_result = single_image.convolve(single_kernel, dims) + potential_biases(j,0);

            memcpy(convolutions.data() +j*output_size_rows_columns +i*output_size_rows_columns*kernels_number,
                   tmp_result.data(), static_cast<size_t>(output_size_rows_columns)*sizeof(float));
        }
    }
}


/// Calculates activations

void ConvolutionalLayer::calculate_activations(type* convolutions, const Tensor<Index, 1>& convolutions_dimensions,
                                               type* activations, const Tensor<Index, 1>& activations_dimensions) const
{
    //@todo debug checks

    switch(activation_function)
    {
    case ActivationFunction::Linear: linear(convolutions, convolutions_dimensions, activations, activations_dimensions); return;

    case ActivationFunction::Logistic: logistic(convolutions, convolutions_dimensions, activations, activations_dimensions); return;

    case ActivationFunction::HyperbolicTangent: hyperbolic_tangent(convolutions, convolutions_dimensions, activations, activations_dimensions); return;

    case ActivationFunction::Threshold: threshold(convolutions, convolutions_dimensions, activations, activations_dimensions); return;

    case ActivationFunction::SymmetricThreshold: symmetric_threshold(convolutions, convolutions_dimensions, activations, activations_dimensions); return;

    case ActivationFunction::RectifiedLinear: rectified_linear(convolutions, convolutions_dimensions, activations, activations_dimensions); return;

    case ActivationFunction::ScaledExponentialLinear: scaled_exponential_linear(convolutions, convolutions_dimensions, activations, activations_dimensions); return;

    case ActivationFunction::SoftPlus: soft_plus(convolutions, convolutions_dimensions, activations, activations_dimensions); return;

    case ActivationFunction::SoftSign: soft_sign(convolutions, convolutions_dimensions, activations, activations_dimensions); return;

    case ActivationFunction::HardSigmoid: hard_sigmoid(convolutions, convolutions_dimensions, activations, activations_dimensions); return;

    case ActivationFunction::ExponentialLinear: exponential_linear(convolutions, convolutions_dimensions, activations, activations_dimensions); return;

    default: return;
    }

}

/// Calculates activations derivatives

void ConvolutionalLayer::calculate_activations_derivatives(type* convolutions, const Tensor<Index, 1>& convolutions_dimensions,
                                                           type* activations, const Tensor<Index, 1>& activations_dimensions,
                                                           type* activations_derivatives, const Tensor<Index, 1>& activations_derivatives_dimensions) const
{
    // @todo debug checks

    switch(activation_function)
    {
    case ActivationFunction::Linear: linear_derivatives(convolutions, convolutions_dimensions, activations, activations_dimensions, activations_derivatives, activations_derivatives_dimensions); return;

    case ActivationFunction::Logistic: logistic_derivatives(convolutions, convolutions_dimensions, activations, activations_dimensions, activations_derivatives, activations_derivatives_dimensions); return;

    case ActivationFunction::HyperbolicTangent: hyperbolic_tangent_derivatives(convolutions, convolutions_dimensions, activations, activations_dimensions, activations_derivatives, activations_derivatives_dimensions); return;

    case ActivationFunction::Threshold: threshold_derivatives(convolutions, convolutions_dimensions, activations, activations_dimensions, activations_derivatives, activations_derivatives_dimensions); return;

    case ActivationFunction::SymmetricThreshold: symmetric_threshold_derivatives(convolutions, convolutions_dimensions, activations, activations_dimensions, activations_derivatives, activations_derivatives_dimensions); return;

    case ActivationFunction::RectifiedLinear: rectified_linear_derivatives(convolutions, convolutions_dimensions, activations, activations_dimensions, activations_derivatives, activations_derivatives_dimensions); return;

    case ActivationFunction::ScaledExponentialLinear: scaled_exponential_linear_derivatives(convolutions, convolutions_dimensions, activations, activations_dimensions, activations_derivatives, activations_derivatives_dimensions); return;

    case ActivationFunction::SoftPlus: soft_plus_derivatives(convolutions, convolutions_dimensions, activations, activations_dimensions, activations_derivatives, activations_derivatives_dimensions); return;

    case ActivationFunction::SoftSign: soft_sign_derivatives(convolutions, convolutions_dimensions, activations, activations_dimensions, activations_derivatives, activations_derivatives_dimensions); return;

    case ActivationFunction::HardSigmoid: hard_sigmoid_derivatives(convolutions, convolutions_dimensions, activations, activations_dimensions, activations_derivatives, activations_derivatives_dimensions); return;

    case ActivationFunction::ExponentialLinear: exponential_linear_derivatives(convolutions, convolutions_dimensions, activations, activations_dimensions, activations_derivatives, activations_derivatives_dimensions); return;

    default: return;
    }
}


void ConvolutionalLayer::forward_propagate(type* inputs_data,
                                           const Tensor<Index,1>& inputs_dimensions,
                                           LayerForwardPropagation* layer_forward_propagation,
                                           const bool& switch_train)
{

    ConvolutionalLayerForwardPropagation* convolutional_layer_forward_propagation
            = static_cast<ConvolutionalLayerForwardPropagation*>(layer_forward_propagation);

    const Index batch_samples_number = layer_forward_propagation->batch_samples_number;

    const Index inputs_channels_number = get_inputs_channels_number();
    const Index inputs_rows_number = get_inputs_rows_number();
    const Index inputs_columns_number = get_inputs_columns_number();

    const TensorMap<Tensor<type, 4>> inputs(inputs_data,
                                            batch_samples_number,
                                            inputs_channels_number,
                                            inputs_rows_number,
                                            inputs_columns_number);

    type* convolutions_data = convolutional_layer_forward_propagation->get_convolutions_data();

    calculate_convolutions(inputs,
                           convolutions_data);

     const Tensor<Index, 1> outputs_dimensions = get_outputs_dimensions();

    type* outputs_data = convolutional_layer_forward_propagation->outputs_data;

    if(switch_train) // Perform training
    {
        calculate_activations_derivatives(convolutions_data,
                                          outputs_dimensions,
                                          outputs_data,
                                          outputs_dimensions,
                                          convolutional_layer_forward_propagation->get_activations_derivatives_data(),
                                          outputs_dimensions);

    }
    else // Perform deployment
    {
        calculate_activations(convolutions_data,
                              outputs_dimensions,
                              outputs_data,
                              outputs_dimensions);
    }

}


void ConvolutionalLayer::calculate_hidden_delta(LayerForwardPropagation* next_layer_forward_propagation,
                                                LayerBackPropagation* next_layer_back_propagation,
                                                LayerBackPropagation* this_layer_back_propagation) const
{
    ConvolutionalLayerBackPropagation* this_convolutional_layer_back_propagation =
            static_cast<ConvolutionalLayerBackPropagation*>(this_layer_back_propagation);

    switch(next_layer_back_propagation->layer_pointer->get_type())
    {
    case Type::Flatten:
    {

        FlattenLayerForwardPropagation* next_flatten_layer_forward_propagation =
                static_cast<FlattenLayerForwardPropagation*>(next_layer_forward_propagation);

        FlattenLayerBackPropagation* next_flatten_layer_back_propagation =
                static_cast<FlattenLayerBackPropagation*>(next_layer_back_propagation);

        calculate_hidden_delta(next_flatten_layer_forward_propagation,
                               next_flatten_layer_back_propagation,
                               this_convolutional_layer_back_propagation);
    }
        break;
    default:
    {
        cout << "Neural network structure not implemented: " << next_layer_back_propagation->layer_pointer->get_type_string() << endl;
        return;
    }
    }
}


void ConvolutionalLayer::calculate_hidden_delta(FlattenLayerForwardPropagation* next_flatten_layer_forward_propagation,
                                                FlattenLayerBackPropagation* next_flatten_layer_back_propagation,
                                                ConvolutionalLayerBackPropagation* convolutional_layer_back_propagation) const
{
    const Index batch_samples_number = convolutional_layer_back_propagation->batch_samples_number;

    const Index neurons_number = get_neurons_number();

    memcpy(convolutional_layer_back_propagation->deltas_data,
           next_flatten_layer_back_propagation->deltas_data,
           batch_samples_number*neurons_number);
}


void ConvolutionalLayer::calculate_error_gradient(type* input_data,
                                                  LayerForwardPropagation* forward_propagation,
                                                  LayerBackPropagation* back_propagation) const
{
    const Index batch_samples_number = back_propagation->batch_samples_number;
    const Index inputs_channels_number = get_inputs_channels_number();
    const Index inputs_rows_number = get_inputs_rows_number();
    const Index inputs_columns_number = get_inputs_columns_number();

    const Index kernels_number = get_kernels_number();
    const Index kernels_channels_number = get_kernels_channels_number();
    const Index kernels_rows_number = get_kernels_rows_number();
    const Index kernels_columns_number = get_kernels_columns_number();

    const Index outputs_rows_number = get_outputs_rows_number();
    const Index outputs_columns_number = get_outputs_columns_number();

    const Index image_size = inputs_channels_number*inputs_rows_number*inputs_columns_number;

    const ConvolutionalLayerForwardPropagation* convolutional_layer_forward_propagation =
            static_cast<ConvolutionalLayerForwardPropagation*>(forward_propagation);

    ConvolutionalLayerBackPropagation* convolutional_layer_back_propagation =
            static_cast<ConvolutionalLayerBackPropagation*>(back_propagation);

    const TensorMap<Tensor<type,4>> inputs(input_data,
                                     batch_samples_number,
                                     inputs_channels_number,
                                     inputs_rows_number,
                                     inputs_columns_number);

    const TensorMap<Tensor<type, 4>> deltas(back_propagation->deltas_data,
                                            batch_samples_number,
                                            kernels_number,
                                            outputs_rows_number,
                                            outputs_columns_number);

    convolutional_layer_back_propagation->deltas_times_activations_derivatives.device(*thread_pool_device)
            = deltas * convolutional_layer_forward_propagation->activations_derivatives;

    Tensor<type, 4> delta_slice;

    const Index delta_slice_dimensions = outputs_rows_number*outputs_columns_number;

    Eigen::array<Eigen::Index, 2> offsets;
    Eigen::array<Eigen::Index, 2> extents;

    // Biases derivatives

    for(Index kernel_index = 0; kernel_index < kernels_number; kernel_index++)
    {
        offsets = {kernel_index, delta_slice_dimensions*kernel_index};
        extents = {2, delta_slice_dimensions};

        // delta_slice;// @todo compilation error because change of dimensions !!!
        //= deltas_times_activations_derivatives.slice(offsets, extents);

        const Tensor<type, 0> current_sum = delta_slice.sum();

        convolutional_layer_back_propagation->biases_derivatives(kernel_index) = current_sum();
    }

    // Synaptic weights derivatives

    type* synaptic_weights_derivatives_data = convolutional_layer_back_propagation->synaptic_weights_derivatives.data();

    const Index kernel_synaptic_weights_number = kernels_channels_number*kernels_rows_number*kernels_columns_number;

    for(Index kernel_index = 0; kernel_index < kernels_number; kernel_index++)
    {
        TensorMap<Tensor<type, 3>> kernel_synaptic_weights_derivatives(synaptic_weights_derivatives_data + kernel_index*kernel_synaptic_weights_number,
                                                                       kernels_channels_number,
                                                                       kernels_rows_number,
                                                                       kernels_columns_number);

        for(Index image_index = 0; image_index < batch_samples_number; image_index++)
        {
            offsets = {image_index, delta_slice_dimensions*kernel_index};
            extents = {1, delta_slice_dimensions};

            // delta_slice; // @todo compilation error because change of dimensions !!! = deltas_times_derivatives.slice(offsets, extents);
            //  = deltas_times_derivatives.slice(offsets, extents);

            const TensorMap<Tensor<type, 3>> image(inputs.data() + image_index*image_size,
                                                   inputs_channels_number,
                                                   inputs_rows_number,
                                                   inputs_columns_number);

            const TensorMap<Tensor<type, 3>> delta_reshape(delta_slice.data(),
                                                           1, // inputs_channels_number?
                                                           outputs_rows_number,
                                                           outputs_columns_number);

            if(image_index == 0)
            {
                kernel_synaptic_weights_derivatives = image.convolve(delta_reshape, convolution_dimensions);
            }
            else
            {
                kernel_synaptic_weights_derivatives += image.convolve(delta_reshape, convolution_dimensions);
            }
        }

        memcpy(synaptic_weights_derivatives_data + kernel_synaptic_weights_number*kernel_index,
               kernel_synaptic_weights_derivatives.data(),
               static_cast<size_t>(kernel_synaptic_weights_number)*sizeof(type));
    }
}


void ConvolutionalLayer::insert_gradient(LayerBackPropagation* back_propagation, const Index& index, Tensor<type, 1>& gradient) const
{
    ConvolutionalLayerBackPropagation* convolutional_layer_back_propagation =
            static_cast<ConvolutionalLayerBackPropagation*>(back_propagation);

    const Index biases_number = get_biases_number();
    const Index synaptic_weights_number = get_synaptic_weights_number();

    type* biases_derivatives_data = convolutional_layer_back_propagation->biases_derivatives.data();
    type* synaptic_weights_derivatives_data = convolutional_layer_back_propagation->synaptic_weights_derivatives.data();

    copy(biases_derivatives_data,
         biases_derivatives_data + biases_number,
         gradient.data() + index);

    copy(synaptic_weights_derivatives_data,
         synaptic_weights_derivatives_data + synaptic_weights_number,
         gradient.data() + index + biases_number);
}


/// Returns the convolutional layer's activation function.

ConvolutionalLayer::ActivationFunction ConvolutionalLayer::get_activation_function() const
{
    return activation_function;
}


/// Returns a string with the name of the layer activation function.
/// This can be Logistic, HyperbolicTangent, Threshold, SymmetricThreshold, Linear, RectifiedLinear, ScaledExponentialLinear.

string ConvolutionalLayer::write_activation_function() const
{
    switch(activation_function)
    {
    case ActivationFunction::Logistic:
        return "Logistic";

    case ActivationFunction::HyperbolicTangent:
        return "HyperbolicTangent";

    case ActivationFunction::Threshold:
        return "Threshold";

    case ActivationFunction::SymmetricThreshold:
        return "SymmetricThreshold";

    case ActivationFunction::Linear:
        return "Linear";

    case ActivationFunction::RectifiedLinear:
        return "RectifiedLinear";

    case ActivationFunction::ScaledExponentialLinear:
        return "ScaledExponentialLinear";

    case ActivationFunction::SoftPlus:
        return "SoftPlus";

    case ActivationFunction::SoftSign:
        return "SoftSign";

    case ActivationFunction::HardSigmoid:
        return "HardSigmoid";

    case ActivationFunction::ExponentialLinear:
        return "ExponentialLinear";
    }

    return string();
}


/// Returns the number of rows the result of applying the layer's kernels to an image will have.

Index ConvolutionalLayer::get_outputs_rows_number() const
{
    const Index inputs_rows_number = get_inputs_rows_number();
    const Index kernels_rows_number = get_kernels_rows_number();

    return inputs_rows_number - kernels_rows_number + 1;

    ///@todo padding

    //    const Index kernels_rows_number = get_kernels_rows_number();

    //    const Index padding_height = get_padding_height();

    //    return ((input_variables_dimensions(2) - kernels_rows_number + 2 * padding_height)/row_stride) + 1;
}


/// Returns the number of columns the result of applying the layer's kernels to an image will have.

Index ConvolutionalLayer::get_outputs_columns_number() const
{
    const Index inputs_columns_number = get_inputs_columns_number();

    const Index kernels_columns_number = get_kernels_columns_number();

    return inputs_columns_number - kernels_columns_number + 1;

    ///@todo padding

    //    const Index kernels_columns_number = get_kernels_columns_number();

    //    const Index padding_width = get_padding_width();

    //    return ((input_variables_dimensions(3) - kernels_columns_number + 2 * padding_width)/column_stride) + 1;
}


/// Returns the dimension of the input variables

Tensor<Index, 1> ConvolutionalLayer::get_inputs_dimensions() const
{
    return inputs_dimensions;
}


/// Returns a vector containing the number of channels, rows and columns of the result of applying the layer's kernels to an image.

Tensor<Index, 1> ConvolutionalLayer::get_outputs_dimensions() const
{
    Tensor<Index, 1> outputs_dimensions(3);

    outputs_dimensions(0) = get_kernels_number();
    outputs_dimensions(1) = get_outputs_rows_number();
    outputs_dimensions(2) = get_outputs_columns_number();

    return outputs_dimensions;
}


/// Returns the padding option.

ConvolutionalLayer::ConvolutionType ConvolutionalLayer::get_convolution_type() const
{
    return convolution_type;
}


/// Returns a string with the name of the convolution type.
/// This can be Valid and Same.

string ConvolutionalLayer::write_convolution_type() const
{
    switch(convolution_type)
    {
    case ConvolutionType::Valid:
        return "Valid";

    case ConvolutionType::Same:
        return "Same";
    }

    return string();
}


/// Returns the column stride.

Index ConvolutionalLayer::get_column_stride() const
{
    return column_stride;
}


/// Returns the row stride.

Index ConvolutionalLayer::get_row_stride() const
{
    return row_stride;
}


///Returns the number of kernels of the layer.

Index ConvolutionalLayer::get_kernels_number() const
{
    return synaptic_weights.dimension(0);
}


/// Returns the number of channels of the layer's kernels.

Index ConvolutionalLayer::get_kernels_channels_number() const
{
    return synaptic_weights.dimension(1);
}


/// Returns the number of rows of the layer's kernels.

Index  ConvolutionalLayer::get_kernels_rows_number() const
{
    return synaptic_weights.dimension(2);
}


/// Returns the number of columns of the layer's kernels.

Index ConvolutionalLayer::get_kernels_columns_number() const
{
    return synaptic_weights.dimension(3);
}


/// Returns the total number of columns of zeroes to be added to an image before applying a kernel, which depends on the padding option set.

Index ConvolutionalLayer::get_padding_width() const
{
    switch(convolution_type)
    {
    case ConvolutionType::Valid:
    {
        return 0;
    }

    case ConvolutionType::Same:
    {
        return column_stride*(inputs_dimensions[2] - 1) - inputs_dimensions[2] + get_kernels_columns_number();
    }
    }

    return 0;
}


/// Returns the total number of rows of zeros to be added to an image before applying a kernel, which depends on the padding option set.

Index ConvolutionalLayer::get_padding_height() const
{
    switch(convolution_type)
    {
    case ConvolutionType::Valid:
    {
        return 0;
    }

    case ConvolutionType::Same:
    {
        return row_stride*(inputs_dimensions[1] - 1) - inputs_dimensions[1] + get_kernels_rows_number();
    }
    }

    return 0;
}


/// Returns the number of inputs

Index ConvolutionalLayer::get_inputs_number() const
{
    return get_inputs_channels_number() * get_inputs_rows_number() * get_inputs_columns_number();
}


/// Returns the number of neurons

Index ConvolutionalLayer::get_neurons_number() const
{
    const Index kernels_number = get_kernels_number();
    const Index kernels_rows_number = get_kernels_rows_number();
    const Index kernels_columns_number = get_kernels_columns_number();

    return kernels_number*kernels_rows_number*kernels_columns_number;
}


/// Returns the layer's parameters in the form of a vector.

Tensor<type, 1> ConvolutionalLayer::get_parameters() const
{
    Tensor<type, 1> parameters(get_parameters_number());

    memcpy(parameters.data(),
           biases.data(), static_cast<size_t>(biases.size())*sizeof(float));

    memcpy(parameters.data() + biases.size(),
           synaptic_weights.data(), static_cast<size_t>(synaptic_weights.size())*sizeof(float));

    return parameters;
}


/// Returns the number of parameters of the layer.

Index ConvolutionalLayer::get_parameters_number() const
{
    return synaptic_weights.size() + biases.size();
}


/// Sets and initializes the layer's parameters in accordance with the dimensions taken as input.
/// The initialization values are random values from a normal distribution.
/// @todo change to memcpy approach
/// @param new_inputs_dimensions A vector containing the desired inputs' dimensions (number of images, number of channels, rows number, columns number).
/// @param new_kernels_dimensions A vector containing the desired kernels' dimensions (number of kernels, number of channels, rows number, columns number).

void ConvolutionalLayer::set(const Tensor<Index, 1>& new_inputs_dimensions, const Tensor<Index, 1>& new_kernels_dimensions)
{
#ifdef OPENNN_DEBUG

    const Index inputs_dimensions_number = new_inputs_dimensions.size();

    if(inputs_dimensions_number != 4)
    {
        ostringstream buffer;
        buffer << "OpenNN Exception: ConvolutionalLayer class.\n"
               << "ConvolutionalLayer(const Tensor<Index, 1>&) constructor.\n"
               << "Number of inputs dimensions (" << inputs_dimensions_number << ") must be 4 (number of images, channels, rows, columns).\n";

        throw invalid_argument(buffer.str());
    }

    const Index kernels_dimensions_number = new_kernels_dimensions.size();

    if(kernels_dimensions_number != 4)
    {
        ostringstream buffer;

        buffer << "OpenNN Exception: ConvolutionalLayer class.\n"
               << "void set(const Tensor<Index, 1>&) method.\n"
               << "Number of kernels dimensions (" << kernels_dimensions_number << ") must be 4 (number of images, kernels, rows, columns).\n";

        throw invalid_argument(buffer.str());
    }

#endif

    const Index kernels_number = new_kernels_dimensions[0];
    const Index kernels_channels_number = new_inputs_dimensions[1];
    const Index kernels_rows_number = new_kernels_dimensions[2];
    const Index kernels_columns_number = new_kernels_dimensions[3];

    biases.resize(kernels_number);
    biases.setRandom();

    synaptic_weights.resize(kernels_number,
                            kernels_channels_number,
                            kernels_rows_number,
                            kernels_columns_number);

    synaptic_weights.setRandom();

    inputs_dimensions = new_inputs_dimensions;
}


void ConvolutionalLayer::set_name(const string& new_layer_name)
{
    layer_name = new_layer_name;
}


/// Initializes the layer's biases to a given value.
/// @param value The desired value.

void ConvolutionalLayer::set_biases_constant(const type& value)
{
    biases.setConstant(value);
}


/// Initializes the layer's synaptic weights to a given value.
/// @param value The desired value.

void ConvolutionalLayer::set_synaptic_weights_constant(const type& value)
{
    synaptic_weights.setConstant(value);
}


/// Initializes the layer's parameters to a given value.
/// @param value The desired value.

void ConvolutionalLayer::set_parameters_constant(const type& value)
{
    set_biases_constant(value);

    set_synaptic_weights_constant(value);
}


/// Sets the parameters to random numbers using Eigen's setRandom.

void ConvolutionalLayer::set_parameters_random()
{
    biases.setRandom();

    synaptic_weights.setRandom();
}


/// Sets the layer's activation function.
/// @param new_activation_function The desired activation function.

void ConvolutionalLayer::set_activation_function(const ConvolutionalLayer::ActivationFunction& new_activation_function)
{
    activation_function = new_activation_function;
}

/// Sets a new activation(or transfer) function in a single layer.
/// The second argument is a string containing the name of the function("Logistic", "HyperbolicTangent", "Threshold", etc).
/// @param new_activation_function Activation function for that layer.

void ConvolutionalLayer::set_activation_function(const string& new_activation_function_name)
{

    if(new_activation_function_name == "Logistic")
    {
        activation_function = ActivationFunction::Logistic;
    }
    else if(new_activation_function_name == "HyperbolicTangent")
    {
        activation_function = ActivationFunction::HyperbolicTangent;
    }
    else if(new_activation_function_name == "Threshold")
    {
        activation_function = ActivationFunction::Threshold;
    }
    else if(new_activation_function_name == "SymmetricThreshold")
    {
        activation_function = ActivationFunction::SymmetricThreshold;
    }
    else if(new_activation_function_name == "Linear")
    {
        activation_function = ActivationFunction::Linear;
    }
    else if(new_activation_function_name == "RectifiedLinear")
    {
        activation_function = ActivationFunction::RectifiedLinear;
    }
    else if(new_activation_function_name == "ScaledExponentialLinear")
    {
        activation_function = ActivationFunction::ScaledExponentialLinear;
    }
    else if(new_activation_function_name == "SoftPlus")
    {
        activation_function = ActivationFunction::SoftPlus;
    }
    else if(new_activation_function_name == "SoftSign")
    {
        activation_function = ActivationFunction::SoftSign;
    }
    else if(new_activation_function_name == "HardSigmoid")
    {
        activation_function = ActivationFunction::HardSigmoid;
    }
    else if(new_activation_function_name == "ExponentialLinear")
    {
        activation_function = ActivationFunction::ExponentialLinear;
    }
    else
    {
        ostringstream buffer;

        buffer << "OpenNN Exception: ConvolutionalLayer class.\n"
               << "void set_activation_function(const string&) method.\n"
               << "Unknown activation function: " << new_activation_function_name << ".\n";

        throw invalid_argument(buffer.str());
    }
}


/// Sets the layer's biases.
/// @param new_biases The desired biases.

void ConvolutionalLayer::set_biases(const Tensor<type, 1>& new_biases)
{
    biases = new_biases;
}


/// Sets the layer's synaptic weights.
/// @param new_synaptic_weights The desired synaptic weights.

void ConvolutionalLayer::set_synaptic_weights(const Tensor<type, 4>& new_synaptic_weights)
{
    synaptic_weights = new_synaptic_weights;
}


/// Sets the padding option.
/// @param new_convolution_type The desired convolution type.

void ConvolutionalLayer::set_convolution_type(const ConvolutionalLayer::ConvolutionType& new_convolution_type)
{
    convolution_type = new_convolution_type;
}


/// Sets the padding option.
/// @param new_convolution_type The desired convolution type.
void ConvolutionalLayer::set_convolution_type(const string& new_convolution_type)
{
    if(new_convolution_type == "Valid")
    {
        convolution_type = ConvolutionType::Valid;
    }
    else if(new_convolution_type == "Same")
    {
        convolution_type = ConvolutionType::Same;
    }
    else
    {
        ostringstream buffer;

        buffer << "OpenNN Exception: ConvolutionalLayer class.\n"
               << "void set_convolution_type(const string&) method.\n"
               << "Unknown convolution type: " << new_convolution_type << ".\n";

        throw invalid_argument(buffer.str());
    }
}

/// Sets the kernels' row stride.
/// @param new_stride_row The desired row stride.

void ConvolutionalLayer::set_row_stride(const Index& new_stride_row)
{
    if(new_stride_row <= 0)
    {
        throw ("EXCEPTION: new_stride_row must be a positive number");
    }

    row_stride = new_stride_row;
}


/// Sets the kernels' column stride.
/// @param new_stride_row The desired column stride.

void ConvolutionalLayer::set_column_stride(const Index& new_stride_column)
{
    if(new_stride_column <= 0)
    {
        throw ("EXCEPTION: new_stride_column must be a positive number");
    }

    column_stride = new_stride_column;
}

void ConvolutionalLayer::set_inputs_dimenisons(const Tensor<Index,1>& new_inputs_dimensions)
{
    inputs_dimensions = new_inputs_dimensions;
}


/// Sets the synaptic weights and biases to the given values.
/// @param new_parameters A vector containing the synaptic weights and biases, in this order.

void ConvolutionalLayer::set_parameters(const Tensor<type, 1>& new_parameters, const Index& )
{
    const Index kernels_number = get_kernels_number();
    const Index kernels_channels_number = get_kernels_channels_number();
    const Index kernels_rows_number = get_kernels_rows_number();
    const Index kernels_columns_number = get_kernels_columns_number();

    synaptic_weights.resize(kernels_number,
                            kernels_channels_number,
                            kernels_rows_number,
                            kernels_columns_number);

    biases.resize(kernels_number);

    memcpy(biases.data(),
           new_parameters.data(),
           static_cast<size_t>(kernels_number)*sizeof(type));

    memcpy(synaptic_weights.data(),
           new_parameters.data() + biases.size(),
           static_cast<size_t>(synaptic_weights.size())*sizeof(type));
}

/// Returns the number of biases in the layer.

Index ConvolutionalLayer::get_biases_number() const
{
    return biases.size();
}


/// Returns the number of synaptic weights in the layer.

Index ConvolutionalLayer::get_synaptic_weights_number() const
{
    return synaptic_weights.size();
}


/// Returns the number of channels of the input.

Index ConvolutionalLayer::get_inputs_channels_number() const
{
    return inputs_dimensions[0];
}


/// Returns the number of rows of the input.

Index ConvolutionalLayer::get_inputs_rows_number() const
{
    return inputs_dimensions[1];
}


/// Returns the number of columns of the input.

Index ConvolutionalLayer::get_inputs_columns_number() const
{
    return inputs_dimensions[2];
}


/// Serializes the convolutional layer object into an XML document of the TinyXML.
/// See the OpenNN manual for more information about the format of this document.

void ConvolutionalLayer::write_XML(tinyxml2::XMLPrinter& file_stream) const
{
    ostringstream buffer;

    // Convolutional layer

    file_stream.OpenElement("ConvolutionalLayer");

    // Layer name

    file_stream.OpenElement("LayerName");

    buffer.str("");
    buffer << layer_name;

    file_stream.PushText(buffer.str().c_str());

    file_stream.CloseElement();

    // Image size

    file_stream.OpenElement("InputsVariablesDimensions");

    buffer.str("");

    for(Index i = 0; i < inputs_dimensions.size(); i++)
    {
        buffer << inputs_dimensions(i);

        if(i != inputs_dimensions.size() - 1) buffer << " ";
    }

    cout << "Input variables dimensions string: " << buffer.str().c_str() << endl;

    file_stream.PushText(buffer.str().c_str());

    file_stream.CloseElement();

    // Filters number

    file_stream.OpenElement("FiltersNumber");

    buffer.str("");
    buffer << get_kernels_number();

    file_stream.PushText(buffer.str().c_str());

    file_stream.CloseElement();

    // Filters size

    file_stream.OpenElement("FiltersSize");

    buffer.str("");
    buffer << get_kernels_columns_number();

    file_stream.PushText(buffer.str().c_str());

    file_stream.CloseElement();

    // Activation function

    file_stream.OpenElement("ActivationFunction");

    file_stream.PushText(write_activation_function().c_str());

    file_stream.CloseElement();

    // Parameters

    file_stream.OpenElement("Parameters");

    buffer.str("");
    buffer << get_parameters();

    file_stream.PushText(buffer.str().c_str());

    file_stream.CloseElement();

    file_stream.CloseElement();
}


/// Deserializes a TinyXML document into this convolutional layer object.
/// @param document TinyXML document containing the member data.

void ConvolutionalLayer::from_XML(const tinyxml2::XMLDocument& document)
{
    ostringstream buffer;

    // Convolution layer

    const tinyxml2::XMLElement* convolutional_layer_element = document.FirstChildElement("ConvolutionalLayer");

    if(!convolutional_layer_element)
    {
        buffer << "OpenNN Exception: ConvolutionalLayer class.\n"
               << "void from_XML(const tinyxml2::XMLDocument&) method.\n"
               << "Convolutional layer element is nullptr.\n";

        throw invalid_argument(buffer.str());
    }

    // Convolutional layer name element

    const tinyxml2::XMLElement* convolution_type_element = convolutional_layer_element->FirstChildElement("LayerName");

    if(!convolution_type_element)
    {
        buffer << "OpenNN Exception: ConvolutionalLayer class.\n"
               << "void from_XML(const tinyxml2::XMLDocument&) method.\n"
               << "Convolution type element is nullptr.\n";

        throw invalid_argument(buffer.str());
    }

    const string convolution_type_string = convolution_type_element->GetText();

    set_convolution_type(convolution_type_string);

    // Input variables dimensions element

    const tinyxml2::XMLElement* input_variables_dimensions_element = convolutional_layer_element->FirstChildElement("InputVariablesDimensions");

    if(!input_variables_dimensions_element)
    {
        buffer << "OpenNN Exception: ConvolutionalLayer class.\n"
               << "void from_XML(const tinyxml2::XMLDocument&) method.\n"
               << "Convolutional input variables dimensions element is nullptr.\n";

        throw invalid_argument(buffer.str());
    }

    const string input_variables_dimensions_string = input_variables_dimensions_element->GetText();

    //    set_input_variables_dimenisons(static_cast<Index>(stoi(input_variables_dimensions_string));

    // Filters Number element

    const tinyxml2::XMLElement* filters_number_element = input_variables_dimensions_element->FirstChildElement("FiltersNumber");

    if(!filters_number_element)
    {
        buffer << "OpenNN Exception: ConvolutionalLayer class.\n"
               << "void from_XML(const tinyxml2::XMLDocument&) method.\n"
               << "Convolutional filters number element is nullptr.\n";

        throw invalid_argument(buffer.str());
    }

    const string filters_number_element_string = filters_number_element->GetText();

    //    set_input_variables_dimenisons(static_cast<Index>(stoi(input_variables_dimensions_string));

    // Column stride

    const tinyxml2::XMLElement* filters_size_element = convolutional_layer_element->FirstChildElement("FiltersSize");

    if(!filters_size_element)
    {
        buffer << "OpenNN Exception: ConvolutionalLayer class.\n"
               << "void from_XML(const tinyxml2::XMLDocument&) method.\n"
               << "Convolutional filters size element is nullptr.\n";

        throw invalid_argument(buffer.str());
    }

    const string filters_size_element_string = filters_size_element->GetText();

    //    set_column_stride(static_cast<Index>(stoi(filters_size_element_string)));

    // Row stride

    const tinyxml2::XMLElement* activation_function_element = convolutional_layer_element->FirstChildElement("ActivationFunction");

    if(!activation_function_element)
    {
        buffer << "OpenNN Exception: ConvolutionalLayer class.\n"
               << "void from_XML(const tinyxml2::XMLDocument&) method.\n"
               << "Convolutional activation function element is nullptr.\n";

        throw invalid_argument(buffer.str());
    }

    const string activation_function_string = activation_function_element->GetText();

    set_activation_function(activation_function_string);

    // Parameters

    const tinyxml2::XMLElement* parameters_element = convolutional_layer_element->FirstChildElement("Parameters");

    if(!parameters_element)
    {
        buffer << "OpenNN Exception: ConvolutionalLayer class.\n"
               << "void from_XML(const tinyxml2::XMLDocument&) method.\n"
               << "Parameters element is nullptr.\n";

        throw invalid_argument(buffer.str());
    }

    if(parameters_element->GetText())
    {
        const string parameters_string = parameters_element->GetText();

        set_parameters(to_type_vector(parameters_string, ' '));
    }
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
