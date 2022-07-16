#include "raycaster.h"

#include <stdlib.h>

void RayCasterDestruct(RayCaster *rayCaster);

RayCaster *RayCasterConstruct(void)
{
    RayCaster *rayCaster = (RayCaster *) malloc(sizeof(RayCaster));
    if (!rayCaster) {
        return NULL;
    }
    rayCaster->derived = NULL;

    rayCaster->Start = NULL;
    rayCaster->Trace = NULL;
    rayCaster->Destruct = RayCasterDestruct;

    return rayCaster;
}

void RayCasterDestruct(RayCaster *rayCaster)
{
    free(rayCaster);
}