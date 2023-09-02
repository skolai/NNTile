# @copyright (c) 2022-2023 Skolkovo Institute of Science and Technology
#                           (Skoltech). All rights reserved.
#
# NNTile is software framework for fast training of big neural networks on
# distributed-memory heterogeneous systems based on StarPU runtime system.
#
# @file wrappers/python/nntile/layer/add_slice.py
# Add slice layer of NNTile Python package
#
# @version 1.0.0
# @author Aleksandr Mikhalev
# @author Aleksandr Katrutsa
# @date 2023-07-16

from .base_layer import BaseLayer
from nntile.tensor import add_async, copy_async, add_slice_async, sum_slice_async
from nntile.tensor import TensorTraits, TensorMoments

class AddSlice(BaseLayer):

    def __init__(self, x: TensorMoments, y: TensorMoments, u: TensorMoments, axis: int):
        super().__init__([x, y], [u], [], [])
        # Set up local named parameters
        self.x = x
        self.y = y
        self.u = u
        self.axis = axis

    # Forward propagation of the add_slice layer
    def forward_async(self):
        # Init Y as a copy of X
        copy_async(self.x.value, self.u.value)
        # Add slice operation
        add_slice_async(1, self.y.value, 1, self.u.value, self.axis)
        self.x.value.wont_use()
        self.y.value.wont_use()
        self.u.value.wont_use()

    def backward_async(self):
        add_async(1, self.u.grad, 1, self.x.grad)
        sum_slice_async(1, self.u.grad, 1, self.y.grad, self.axis)
        self.x.grad.wont_use()
        self.y.grad.wont_use()
        self.u.grad.wont_use()
        
    # Simple generator for the add_slice layer
    @staticmethod
    def generate_simple(x: TensorMoments, y: TensorMoments, axis: int,  next_tag: int):
        # Get traits of X
        u_traits = TensorTraits(x.value.shape, x.value.basetile_shape)
        # Create Y with the same traits and distribution as X
        u_value = type(x.value)(u_traits, x.value.distribution, next_tag)
        next_tag = u_value.next_tag
        u_grad = type(x.value)(u_traits, x.value.distribution, next_tag)
        next_tag = u_grad.next_tag
        u = TensorMoments(u_value, u_grad, True)
        # Create activation layer with all the provided tensors
        layer = AddSlice(x, y, u, axis)
        # Return layer and next tag to be used
        return (layer, next_tag)
