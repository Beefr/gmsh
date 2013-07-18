#include <algorithm>
#include <sstream>
#include <list>

#include "OrientationSort.h"
#include "GroupOfElement.h"

using namespace std;

GroupOfElement::GroupOfElement
(std::multimap<int, const MElement*>::iterator begin,
 std::multimap<int, const MElement*>::iterator end,
 const Mesh& mesh){

  // Get Element //
  list<const MElement*> lst;

  for(; begin != end; begin++)
    lst.push_back(begin->second);

  // Alloc //
  this->mesh = &mesh;
  element = new vector<const MElement*>(lst.begin(), lst.end());

  orientationStat = NULL;
}

GroupOfElement::~GroupOfElement(void){
  delete element;

  if(orientationStat)
    delete orientationStat;
}

void GroupOfElement::
orientAllElements(const Basis& basis){
  // If already oriented, delete old orientation //
  if(orientationStat)
    throw Exception
      ("GroupOfElement already oriented");

  // Sort //
  OrientationSort sortPredicate(basis);
  sort(element->begin(), element->end(), sortPredicate);

  // Get Orientation Stats //
  // Get some Data
  const size_t nOrient  = basis.getReferenceSpace().getNReferenceSpace();
  const size_t nElement = element->size();

  // Init
  orientationStat = new vector<size_t>(nOrient);

  for(size_t i = 0; i < nOrient; i++)
    (*orientationStat)[i] = 0;

  // Compute
  for(size_t i = 0; i < nElement; i++)
    (*orientationStat)
      [basis.getReferenceSpace().getReferenceSpace(*(*element)[i])]++;

  // The last line is cool isn't it :-) ?
}

void GroupOfElement::unoriented(void){
  if(orientationStat)
    delete orientationStat;

  orientationStat = NULL;
}

string GroupOfElement::toString(void) const{
  stringstream stream;

  stream << "***********************************************"
         << endl
         << "* Group Of Element"
         << endl
         << "***********************************************"
         << endl
         << "*                                             *"
         << endl
         << "* This group contains the following Elements: *"
         << endl;

  for(size_t i = 0; i < element->size(); i++)
    stream << "*   -- Element #"
           << mesh->getGlobalId(*(*element)[i])
           << endl;

  stream << "*                                             *"
         << endl
         << "***********************************************"
         << endl;

  if(!orientationStat)
    return stream.str();


  stream << "*                                             *"
         << endl
         << "* This group has the following Orientations:  *"
         << endl;

  for(size_t i = 0; i < orientationStat->size(); i++)
    stream << "*   -- Elements with Orientation " << i << " - "
           << (*orientationStat)[i] << endl;

  stream << "*                                             *"
         << endl
         << "***********************************************"
         << endl;

  return stream.str();
}
