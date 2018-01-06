#ifndef SPDIAGNOSTICISSUESFIXER_H
#define SPDIAGNOSTICISSUESFIXER_H

#include <memory>

#include "spdiagnosticissue.h"

class SPDiagnosticIssuesList;

using namespace std;

class SPDiagnosticIssuesFixer
{
public:
    SPDiagnosticIssuesFixer();

    void fix(shared_ptr<SPDiagnosticIssuesList> diagnosticIssuesList);

    void fixUnusedParameterIssue(shared_ptr<SPDiagnosticIssue> diagnosticIssue);

};

#endif // SPDIAGNOSTICISSUESFIXER_H