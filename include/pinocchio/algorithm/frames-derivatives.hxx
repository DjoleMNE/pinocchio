//
// Copyright (c) 2020 INRIA
//

#ifndef __pinocchio_algorithm_frames_derivatives_hxx__
#define __pinocchio_algorithm_frames_derivatives_hxx__

#include "pinocchio/multibody/visitor.hpp"
#include "pinocchio/algorithm/check.hpp"
#include "pinocchio/algorithm/kinematics-derivatives.hpp"

namespace pinocchio
{
  template<
    typename Scalar,
    int Options,
    template<typename, int> class JointCollectionTpl,
    typename Matrix6xOut1,
    typename Matrix6xOut2>
  void getFrameVelocityDerivatives(
    const ModelTpl<Scalar, Options, JointCollectionTpl> & model,
    const DataTpl<Scalar, Options, JointCollectionTpl> & data,
    const JointIndex joint_id,
    const SE3Tpl<Scalar, Options> & placement,
    const ReferenceFrame rf,
    const Eigen::MatrixBase<Matrix6xOut1> & v_partial_dq,
    const Eigen::MatrixBase<Matrix6xOut2> & v_partial_dv)
  {
    typedef DataTpl<Scalar, Options, JointCollectionTpl> Data;

    typedef typename Data::Matrix6x Matrix6x;
    typedef typename Data::SE3 SE3;
    typedef typename Data::Motion Motion;

    EIGEN_STATIC_ASSERT_SAME_MATRIX_SIZE(Matrix6xOut1, Matrix6x);
    EIGEN_STATIC_ASSERT_SAME_MATRIX_SIZE(Matrix6xOut2, Matrix6x);

    Matrix6xOut1 & v_partial_dq_ = PINOCCHIO_EIGEN_CONST_CAST(Matrix6xOut1, v_partial_dq);
    Matrix6xOut2 & v_partial_dv_ = PINOCCHIO_EIGEN_CONST_CAST(Matrix6xOut2, v_partial_dv);
    getJointVelocityDerivatives(model, data, joint_id, rf, v_partial_dq_, v_partial_dv_);

    typedef typename SizeDepType<1>::template ColsReturn<Matrix6xOut1>::Type ColsBlockOut1;
    typedef MotionRef<ColsBlockOut1> MotionOut1;
    typedef typename SizeDepType<1>::template ColsReturn<Matrix6xOut2>::Type ColsBlockOut2;
    typedef MotionRef<ColsBlockOut2> MotionOut2;

    Motion v_tmp;
    const typename SE3::Vector3 trans = data.oMi[joint_id].rotation() * placement.translation();
    const int colRef = nv(model.joints[joint_id]) + idx_v(model.joints[joint_id]) - 1;
    switch (rf)
    {
    case WORLD:
      // Do nothing
      break;

    case LOCAL_WORLD_ALIGNED:
      for (Eigen::DenseIndex col_id = colRef; col_id >= 0;
           col_id = data.parents_fromRow[(size_t)col_id])
      {
        MotionOut1 m1(v_partial_dq_.col(col_id));
        m1.linear() -= trans.cross(m1.angular());
        MotionOut2 m2(v_partial_dv_.col(col_id));
        m2.linear() -= trans.cross(m2.angular());
      }
      break;

    case LOCAL:
      for (Eigen::DenseIndex col_id = colRef; col_id >= 0;
           col_id = data.parents_fromRow[(size_t)col_id])
      {
        v_tmp = v_partial_dq_.col(col_id);
        MotionOut1(v_partial_dq_.col(col_id)) = placement.actInv(v_tmp);
        v_tmp = v_partial_dv_.col(col_id);
        MotionOut2(v_partial_dv_.col(col_id)) = placement.actInv(v_tmp);
      }
      break;

    default:
      break;
    }
  }

  template<
    typename Scalar,
    int Options,
    template<typename, int> class JointCollectionTpl,
    typename Matrix6xOut1,
    typename Matrix6xOut2,
    typename Matrix6xOut3,
    typename Matrix6xOut4>
  void getFrameAccelerationDerivatives(
    const ModelTpl<Scalar, Options, JointCollectionTpl> & model,
    DataTpl<Scalar, Options, JointCollectionTpl> & data,
    const JointIndex joint_id,
    const SE3Tpl<Scalar, Options> & placement,
    const ReferenceFrame rf,
    const Eigen::MatrixBase<Matrix6xOut1> & v_partial_dq,
    const Eigen::MatrixBase<Matrix6xOut2> & a_partial_dq,
    const Eigen::MatrixBase<Matrix6xOut3> & a_partial_dv,
    const Eigen::MatrixBase<Matrix6xOut4> & a_partial_da)
  {
    typedef DataTpl<Scalar, Options, JointCollectionTpl> Data;

    typedef typename Data::Matrix6x Matrix6x;
    typedef typename Data::SE3 SE3;
    typedef typename Data::Motion Motion;

    EIGEN_STATIC_ASSERT_SAME_MATRIX_SIZE(Matrix6xOut1, Matrix6x);
    EIGEN_STATIC_ASSERT_SAME_MATRIX_SIZE(Matrix6xOut2, Matrix6x);
    EIGEN_STATIC_ASSERT_SAME_MATRIX_SIZE(Matrix6xOut3, Matrix6x);
    EIGEN_STATIC_ASSERT_SAME_MATRIX_SIZE(Matrix6xOut4, Matrix6x);

    PINOCCHIO_CHECK_ARGUMENT_SIZE(v_partial_dq.cols(), model.nv);
    PINOCCHIO_CHECK_ARGUMENT_SIZE(a_partial_dq.cols(), model.nv);
    PINOCCHIO_CHECK_ARGUMENT_SIZE(a_partial_dv.cols(), model.nv);
    PINOCCHIO_CHECK_ARGUMENT_SIZE(a_partial_da.cols(), model.nv);
    assert(model.check(data) && "data is not consistent with model.");

    Matrix6xOut1 & v_partial_dq_ = PINOCCHIO_EIGEN_CONST_CAST(Matrix6xOut1, v_partial_dq);
    Matrix6xOut2 & a_partial_dq_ = PINOCCHIO_EIGEN_CONST_CAST(Matrix6xOut2, a_partial_dq);
    Matrix6xOut3 & a_partial_dv_ = PINOCCHIO_EIGEN_CONST_CAST(Matrix6xOut3, a_partial_dv);
    Matrix6xOut4 & a_partial_da_ = PINOCCHIO_EIGEN_CONST_CAST(Matrix6xOut4, a_partial_da);

    getJointAccelerationDerivatives(
      model, data, joint_id, rf, v_partial_dq_, a_partial_dq_, a_partial_dv_, a_partial_da_);

    typedef typename SizeDepType<1>::template ColsReturn<Matrix6xOut1>::Type ColsBlockOut1;
    typedef MotionRef<ColsBlockOut1> MotionOut1;
    typedef typename SizeDepType<1>::template ColsReturn<Matrix6xOut2>::Type ColsBlockOut2;
    typedef MotionRef<ColsBlockOut2> MotionOut2;
    typedef typename SizeDepType<1>::template ColsReturn<Matrix6xOut3>::Type ColsBlockOut3;
    typedef MotionRef<ColsBlockOut3> MotionOut3;
    typedef typename SizeDepType<1>::template ColsReturn<Matrix6xOut4>::Type ColsBlockOut4;
    typedef MotionRef<ColsBlockOut4> MotionOut4;

    const int colRef = nv(model.joints[joint_id]) + idx_v(model.joints[joint_id]) - 1;
    switch (rf)
    {
    case WORLD:
      // Do nothing
      break;

    case LOCAL_WORLD_ALIGNED: {
      const typename SE3::Vector3 trans = data.oMi[joint_id].rotation() * placement.translation();
      for (Eigen::DenseIndex col_id = colRef; col_id >= 0;
           col_id = data.parents_fromRow[(size_t)col_id])
      {
        MotionOut1 m1(v_partial_dq_.col(col_id));
        m1.linear() -= trans.cross(m1.angular());
        MotionOut2 m2(a_partial_dq_.col(col_id));
        m2.linear() -= trans.cross(m2.angular());
        MotionOut3 m3(a_partial_dv_.col(col_id));
        m3.linear() -= trans.cross(m3.angular());
        MotionOut4 m4(a_partial_da_.col(col_id));
        m4.linear() -= trans.cross(m4.angular());
      }
      break;
    }
    case LOCAL: {
      Motion v_tmp;
      for (Eigen::DenseIndex col_id = colRef; col_id >= 0;
           col_id = data.parents_fromRow[(size_t)col_id])
      {
        v_tmp = v_partial_dq_.col(col_id);
        MotionOut1(v_partial_dq_.col(col_id)) = placement.actInv(v_tmp);
        v_tmp = a_partial_dq_.col(col_id);
        MotionOut2(a_partial_dq_.col(col_id)) = placement.actInv(v_tmp);
        v_tmp = a_partial_dv_.col(col_id);
        MotionOut3(a_partial_dv_.col(col_id)) = placement.actInv(v_tmp);
        v_tmp = a_partial_da_.col(col_id);
        MotionOut4(a_partial_da_.col(col_id)) = placement.actInv(v_tmp);
      }
      break;
    }
    default:
      break;
    }
  }
} // namespace pinocchio

#endif // ifndef __pinocchio_algorithm_frames_derivatives_hxx__
