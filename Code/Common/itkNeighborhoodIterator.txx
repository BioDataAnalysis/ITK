/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkNeighborhoodIterator.txx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

  =========================================================================*/
namespace itk {
  
template<class TPixel, unsigned int VDimension>
void NeighborhoodIterator<TPixel, VDimension>
::SetPixelPointers(const IndexType &offset)
{
  const Iterator _end = this->end();
  unsigned int i;
  Iterator Nit;
  TPixel * Iit;
  unsigned long loop[VDimension];
  const Size<VDimension> size = this->GetSize();
  const unsigned long *OffsetTable = m_Image->GetOffsetTable();
  memset(loop, 0, sizeof(long) * VDimension);
  const Size<VDimension> radius = this->GetRadius();
  
  // Find first "upper-left-corner"  pixel address of neighborhood
  Iit = m_Buffer + m_Image->ComputeOffset(offset);
  
  for (i = 0; i<VDimension; ++i)
    {
      Iit -= radius[i] * OffsetTable[i];
    }
  
  // Compute the rest of the pixel addresses
  for (Nit = this->begin(); Nit != _end; ++Nit)
    {
      *Nit = Iit;
      ++Iit;
      for (i = 0; i <VDimension; ++i)
        {
          loop[i]++;
          if ( loop[i] == size[i] )
            {
              if (i==VDimension-1) break;
              Iit +=  OffsetTable[i+1] - OffsetTable[i] * size[i];
              loop[i]= 0;
            }
          else break;
        }      
    }
}
  
template<class TPixel, unsigned int VDimension>
const NeighborhoodIterator<TPixel, VDimension> &
NeighborhoodIterator<TPixel, VDimension>
::operator++()
{
  unsigned int i;
  Iterator it;
  const Iterator _end = this->end();
  
  // Increment pointers.
  for (it = this->begin(); it < _end; ++it)
    {
      (*it)++;
    }
  if (m_OutputBuffer)
    {
      ++m_OutputBuffer;
    }
  
  // Check loop bounds, wrap & add pointer offsets if needed.
  for (i=0; i<VDimension; ++i)
    {
      m_Loop[i]++;
      if ( m_Loop[i] == m_Bound[i] )
        {
          m_Loop[i]= m_StartIndex[i];
          for (it = this->begin(); it < _end; ++it)
            {
              (*it) += m_WrapOffset[i];
            }
          if (m_OutputBuffer)
            {
              m_OutputBuffer += m_WrapOffset[i]
                + m_OutputWrapOffsetModifier[i];
            }
        }        
      else break;
    }
  return *this;
}

template<class TPixel, unsigned int VDimension>
const NeighborhoodIterator<TPixel, VDimension> &
NeighborhoodIterator<TPixel, VDimension>
::operator--()
{
  unsigned int i;
  Iterator it;
  const Iterator _end = this->end();
  
  // Decrement pointers.
  for (it = this->begin(); it < _end; ++it)
    {
      (*it)--;
    }
  if (m_OutputBuffer)
    {
      --m_OutputBuffer;
    }
  
  // Check loop bounds, wrap & add pointer offsets if needed.
  for (i=0; i<VDimension; ++i)
    {
      m_Loop[i]--;
      if ( m_Loop[i] < m_StartIndex[i] )
        {
          m_Loop[i]= m_Bound[i] - 1;
          for (it = this->begin(); it < _end; ++it)
            {
              (*it) -= m_WrapOffset[i];
            }
          if (m_OutputBuffer)
            {
              m_OutputBuffer -= m_WrapOffset[i]
                + m_OutputWrapOffsetModifier[i];
            }
        }        
      else break;
    }
  return *this;
}

} // namespace itk
