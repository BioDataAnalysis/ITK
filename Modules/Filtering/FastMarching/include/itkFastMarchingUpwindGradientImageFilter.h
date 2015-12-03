/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkFastMarchingUpwindGradientImageFilter_h
#define itkFastMarchingUpwindGradientImageFilter_h

#include "itkFastMarchingImageFilter.h"
#include "itkImage.h"

namespace itk
{
/** \class FastMarchingUpwindGradientImageFilter
 *
 * \brief Generates the upwind gradient field of fast marching arrival times.
 *
 * This filter adds some extra functionality to its base class. While the
 * solution T(x) of the Eikonal equation is being generated by the base class
 * with the fast marching method, the filter generates the upwind gradient
 * vectors of T(x), storing them in an image.
 *
 * Since the Eikonal equation generates the arrival times of a wave travelling
 * at a given speed, the generated gradient vectors can be interpreted as the
 * slowness (1/velocity) vectors of the front (the quantity inside the modulus
 * operator in the Eikonal equation).
 *
 * Gradient vectors are computed using upwind finite differences, that is,
 * information only propagates from points where the wavefront has already
 * passed. This is consistent with how the fast marching method works.
 *
 * One more extra feature is the possibility to define a set of Target points
 * where the propagation stops. This can be used to avoid computing the Eikonal
 * solution for the whole domain.  The front can be stopped either when one
 * Target point is reached or all Target points are reached. The propagation
 * can stop after a time TargetOffset has passed since the stop condition is
 * met. This way the solution is computed a bit downstream the Target points,
 * so that the level sets of T(x) corresponding to the Target are smooth.
 *
 *
 * \author Luca Antiga Ph.D.  Biomedical Technologies Laboratory,
 *                            Bioengineering Department, Mario Negri Institute, Italy.
 *
 * \ingroup ITKFastMarching
 */
template<
  typename TLevelSet,
  typename TSpeedImage = Image< float,  TLevelSet ::ImageDimension > >
class FastMarchingUpwindGradientImageFilter:
  public FastMarchingImageFilter< TLevelSet, TSpeedImage >
{
public:
  /** Standard class typdedefs. */
  typedef FastMarchingUpwindGradientImageFilter             Self;
  typedef FastMarchingImageFilter< TLevelSet, TSpeedImage > Superclass;
  typedef SmartPointer< Self >                              Pointer;
  typedef SmartPointer< const Self >                        ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(FastMarchingUpwindGradientImageFilter, FastMarchingImageFilter);

  /** Inherited typedefs. */
  typedef typename Superclass::LevelSetType           LevelSetType;
  typedef typename Superclass::SpeedImageType         SpeedImageType;
  typedef typename Superclass::LevelSetImageType      LevelSetImageType;
  typedef typename Superclass::LevelSetPointer        LevelSetPointer;
  typedef typename Superclass::SpeedImageConstPointer SpeedImageConstPointer;
  typedef typename Superclass::LabelImageType         LabelImageType;
  typedef typename Superclass::PixelType              PixelType;
  typedef typename Superclass::AxisNodeType           AxisNodeType;
  typedef typename Superclass::NodeType               NodeType;
  typedef typename Superclass::NodeContainer          NodeContainer;
  typedef typename Superclass::NodeContainerPointer   NodeContainerPointer;

  typedef typename Superclass::IndexType         IndexType;
  typedef typename Superclass::OutputSpacingType OutputSpacingType;
  typedef typename Superclass::LevelSetIndexType LevelSetIndexType;

  typedef typename Superclass::OutputPointType PointType;

  /** The dimension of the level set. */
  itkStaticConstMacro(SetDimension, unsigned int, Superclass::SetDimension);

  /** Set the container of Target Points.
   * If a target point is reached, the propagation stops.
   * Trial points are represented as a VectorContainer of LevelSetNodes. */
  void SetTargetPoints(NodeContainer *points)
  {
    m_TargetPoints = points;
    this->Modified();
  }

  /** Get the container of Target Points. */
  NodeContainerPointer GetTargetPoints()
  { return m_TargetPoints; }

  /** Get the container of Reached Target Points. */
  NodeContainerPointer GetReachedTargetPoints()
  { return m_ReachedTargetPoints; }

  /** GradientPixel typedef support. */
  typedef CovariantVector< PixelType,
                           itkGetStaticConstMacro(SetDimension) > GradientPixelType;

  /** GradientImage typedef support. */
  typedef Image< GradientPixelType,
                 itkGetStaticConstMacro(SetDimension) > GradientImageType;

  /** GradientImagePointer typedef support. */
  typedef typename GradientImageType::Pointer GradientImagePointer;

  /** Get the gradient image. */
  GradientImagePointer GetGradientImage() const
  { return m_GradientImage; }

  /** Set the GenerateGradientImage flag. Instrument the algorithm to generate
   * the gradient of the Eikonal equation solution while fast marching. */
  itkSetMacro(GenerateGradientImage, bool);

  /** Get the GenerateGradientImage flag. */
  itkGetConstReferenceMacro(GenerateGradientImage, bool);
  itkBooleanMacro(GenerateGradientImage);

  /** Set how long (in terms of arrival times) after targets are reached the
   * front must stop.  This is useful to ensure that the level set of target
   * arrival time is smooth. */
  itkSetMacro(TargetOffset, double);
  /** Get the TargetOffset ivar. */
  itkGetConstReferenceMacro(TargetOffset, double);

  /** Choose whether the front must stop when the first target has been reached
   * or all targets have been reached.
   */
  itkSetMacro(TargetReachedMode, int);
  itkGetConstReferenceMacro(TargetReachedMode, int);
  void SetTargetReachedModeToNoTargets()
  { this->SetTargetReachedMode(NoTargets); }
  void SetTargetReachedModeToOneTarget()
  { this->SetTargetReachedMode(OneTarget); }
  void SetTargetReachedModeToSomeTargets(SizeValueType numberOfTargets)
  {
    this->SetTargetReachedMode(SomeTargets);
    m_NumberOfTargets = numberOfTargets;
  }

  void SetTargetReachedModeToAllTargets()
  { this->SetTargetReachedMode(AllTargets); }

  /** Get the number of targets. */
  itkGetConstReferenceMacro(NumberOfTargets, SizeValueType);

  /** Get the arrival time corresponding to the last reached target.
   *  If TargetReachedMode is set to NoTargets, TargetValue contains
   *  the last (aka largest) Eikonal solution value generated.
   */
  itkGetConstReferenceMacro(TargetValue, double);

  enum {
    NoTargets,
    OneTarget,
    SomeTargets,
    AllTargets
    };

#ifdef ITK_USE_CONCEPT_CHECKING
  // Begin concept checking
  itkConceptMacro( LevelSetDoubleDivisionOperatorsCheck,
                   ( Concept::DivisionOperators< typename TLevelSet::PixelType, double > ) );
  itkConceptMacro( LevelSetDoubleDivisionAndAssignOperatorsCheck,
                   ( Concept::DivisionAndAssignOperators< typename TLevelSet::PixelType, double > ) );
  // End concept checking
#endif

protected:
  FastMarchingUpwindGradientImageFilter();
  ~FastMarchingUpwindGradientImageFilter(){}
  void PrintSelf(std::ostream & os, Indent indent) const ITK_OVERRIDE;

  virtual void Initialize(LevelSetImageType *) ITK_OVERRIDE;

  void GenerateData() ITK_OVERRIDE;

  virtual void UpdateNeighbors(const IndexType & index,
                               const SpeedImageType *, LevelSetImageType *) ITK_OVERRIDE;

  virtual void ComputeGradient(const IndexType & index,
                               const LevelSetImageType *output,
                               const LabelImageType *labelImage,
                               GradientImageType *gradientImage);

private:
  FastMarchingUpwindGradientImageFilter(const Self &) ITK_DELETE_FUNCTION;
  void operator=(const Self &) ITK_DELETE_FUNCTION;

  NodeContainerPointer m_TargetPoints;
  NodeContainerPointer m_ReachedTargetPoints;

  GradientImagePointer m_GradientImage;

  bool m_GenerateGradientImage;

  double m_TargetOffset;

  int m_TargetReachedMode;

  double m_TargetValue;

  SizeValueType m_NumberOfTargets;
};
} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFastMarchingUpwindGradientImageFilter.hxx"
#endif

#endif