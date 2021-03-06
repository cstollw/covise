/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#include "CoviseRenderObject.h"
#include <cover/coVRMSController.h>
#include <PluginUtil/coSphere.h>
#include <net/message.h>
#include <net/tokenbuffer.h>
#include <do/coDistributedObject.h>
#include <do/coDoPoints.h>
#include <do/coDoSpheres.h>
#include <do/coDoLines.h>
#include <do/coDoPolygons.h>
#include <do/coDoTriangleStrips.h>
#include <do/coDoData.h>
#include <do/coDoIntArr.h>
#include <do/coDoPixelImage.h>
#include <do/coDoSet.h>
#include <do/coDoText.h>
#include <do/coDoTexture.h>
#include <do/coDoUnstructuredGrid.h>
#include <do/coDoStructuredGrid.h>
#include <do/coDoRectilinearGrid.h>
#include <do/coDoUniformGrid.h>
#include <do/coDoGeometry.h>

#include "coVRDistributionManager.h"
#include <cover/coVRPluginSupport.h>

using namespace opencover;
using namespace covise;
#define addInt(x) tb.addBinary((char *)&(x), sizeof(int));
#define addFloat(x) tb.addBinary((char *)&(x), sizeof(float));
#define copyInt(x) memcpy(&x, tb.getBinary(sizeof(int)), sizeof(int));
#define copyFloat(x) memcpy(&x, tb.getBinary(sizeof(float)), sizeof(float));
#define HASOBJ 1
#define HASNORMALS 2
#define HASCOLORS 4
#define HASTEXTURE 8
#define HASVERTEXATTRIBUTE 16

CoviseRenderObject::CoviseRenderObject(const coDistributedObject *co, const std::vector<int> &assignedTo)
    : assignedTo(assignedTo)
{
    int i;
    type[0] = '\0';
    texture = NULL;
    textureCoords = NULL;
    numTC = 0;
    numAttributes = 0;
    attrNames = NULL;
    attributes = NULL;
    size = 0;
    sizeu = 0;
    sizev = 0;
    sizew = 0;
    minx = 0;
    maxx = 0;
    miny = 0;
    maxy = 0;
    minz = 0;
    maxz = 0;
    farr1 = NULL;
    farr2 = NULL;
    farr3 = NULL;
    farr4 = NULL;
    farr5 = NULL;
    iarr1 = NULL;
    iarr2 = NULL;
    COVdobj = NULL;
    COVnormals = NULL;
    COVcolors = NULL;
    COVtexture = NULL;
    COVvertexAttribute = NULL;
    name = NULL;
    objs = NULL;
    geometryObject = NULL;
    normalObject = NULL;
    colorObject = NULL;
    textureObject = NULL;
    vertexAttributeObject = NULL;
    geometryFlag = 0;
    pc = NULL;
    byteData = NULL;
    coviseObject = co;
    cluster = coVRMSController::instance()->isCluster();

    if (cluster && coVRDistributionManager::instance().isActive() && this->assignedTo.empty())
    {
        this->assignedTo = coVRDistributionManager::instance().assign(co);
    }

    if (coVRMSController::instance()->isMaster())
    {
        TokenBuffer tb;
        if (co)
        {

            memcpy(type, co->getType(), 7);
            name = new char[strlen(co->getName()) + 1];
            strcpy(name, co->getName());
            numAttributes = co->getNumAttributes();
            attrNames = new char *[numAttributes];
            attributes = new char *[numAttributes];
            const char **attrs = NULL, **anames = NULL;
            co->getAllAttributes(&anames, &attrs);
            for (int i = 0; i < numAttributes; i++)
            {
                attrNames[i] = new char[strlen(anames[i]) + 1];
                strcpy(attrNames[i], anames[i]);
                attributes[i] = new char[strlen(attrs[i]) + 1];
                strcpy(attributes[i], attrs[i]);
            }
            if (cluster)
            {
                tb.addBinary(type, 7);
                tb << name;
                addInt(numAttributes);
                for (int i = 0; i < numAttributes; i++)
                {
                    tb << attrNames[i];
                    tb << attributes[i];
                }
            }

            //cerr << "Sending Object " << name << " numAttribs: " << numAttributes << endl;
            //cerr << "type " << type << endl;
            if (strcmp(type, "GEOMET") == 0)
            {
                coDoGeometry *geometry = (coDoGeometry *)co;
                COVdobj = geometry->getGeometry();
                COVnormals = geometry->getNormals();
                COVcolors = geometry->getColors();
                COVtexture = geometry->getTexture();
                COVvertexAttribute = geometry->getVertexAttribute();
                if (COVdobj)
                    geometryFlag |= HASOBJ;
                if (COVnormals)
                    geometryFlag |= HASNORMALS;
                if (COVcolors)
                    geometryFlag |= HASCOLORS;
                if (COVtexture)
                    geometryFlag |= HASTEXTURE;
                if (COVvertexAttribute)
                    geometryFlag |= HASVERTEXATTRIBUTE;
                if (cluster)
                {
                    addInt(geometryFlag);
                }
            }
            else if (strcmp(type, "SETELE") == 0)
            {
                coDoSet *set = (coDoSet *)co;
                size = set->getNumElements();
                if (cluster)
                {
                    addInt(size);
                }
            }
            else if (strcmp(type, "POLYGN") == 0)
            {
                coDoPolygons *poly = (coDoPolygons *)co;
                sizeu = poly->getNumPolygons();
                sizev = poly->getNumVertices();
                size = poly->getNumPoints();
                poly->getAddresses(&farr1, &farr2, &farr3, &iarr1, &iarr2);
                if (cluster)
                {
                    addInt(sizeu);
                    addInt(sizev);
                    addInt(size);
                    tb.addBinary((char *)farr1, size * sizeof(float));
                    tb.addBinary((char *)farr2, size * sizeof(float));
                    tb.addBinary((char *)farr3, size * sizeof(float));
                    tb.addBinary((char *)iarr1, sizev * sizeof(int));
                    tb.addBinary((char *)iarr2, sizeu * sizeof(int));
                }
            }
            else if (strcmp(type, "TRIANG") == 0)
            {
                coDoTriangleStrips *strip = (coDoTriangleStrips *)co;
                sizeu = strip->getNumStrips();
                sizev = strip->getNumVertices();
                size = strip->getNumPoints();
                strip->getAddresses(&farr1, &farr2, &farr3, &iarr1, &iarr2);
                if (cluster)
                {
                    addInt(sizeu);
                    addInt(sizev);
                    addInt(size);
                    tb.addBinary((char *)farr1, size * sizeof(float));
                    tb.addBinary((char *)farr2, size * sizeof(float));
                    tb.addBinary((char *)farr3, size * sizeof(float));
                    tb.addBinary((char *)iarr1, sizev * sizeof(int));
                    tb.addBinary((char *)iarr2, sizeu * sizeof(int));
                }
            }
            else if (strcmp(type, "UNIGRD") == 0)
            {
                coDoUniformGrid *ugrid = (coDoUniformGrid *)co;
                ugrid->getGridSize(&sizeu, &sizev, &sizew);
                ugrid->getMinMax(&minx, &maxx, &miny, &maxy, &minz, &maxz);
                if (cluster)
                {
                    addInt(sizeu);
                    addInt(sizev);
                    addInt(sizew);
                    addFloat(minx);
                    addFloat(maxx);
                    addFloat(miny);
                    addFloat(maxy);
                    addFloat(minz);
                    addFloat(maxz);
                }
            }
            else if (strcmp(type, "UNSGRD") == 0)
            {
                coDoUnstructuredGrid *unsgrid = (coDoUnstructuredGrid *)co;
                unsgrid->getGridSize(&sizeu, &sizev, &sizew);
                unsgrid->getAddresses(&iarr2, &iarr1, &farr1, &farr2, &farr3);
                unsgrid->getTypeList(&iarr3);
                if (cluster)
                {
                    addInt(sizeu);
                    addInt(sizev);
                    addInt(sizew);
                    tb.addBinary((char *)farr1, sizew * sizeof(float));
                    tb.addBinary((char *)farr2, sizew * sizeof(float));
                    tb.addBinary((char *)farr3, sizew * sizeof(float));
                    tb.addBinary((char *)iarr1, sizev * sizeof(int));
                    tb.addBinary((char *)iarr2, sizeu * sizeof(int));
                    tb.addBinary((char *)iarr3, sizeu * sizeof(int));
                }
            }
            else if (strcmp(type, "RCTGRD") == 0)
            {
                coDoRectilinearGrid *rgrid = (coDoRectilinearGrid *)co;
                rgrid->getGridSize(&sizeu, &sizev, &sizew);
                rgrid->getAddresses(&farr1, &farr2, &farr3);
                if (cluster)
                {
                    addInt(sizeu);
                    addInt(sizev);
                    addInt(sizew);
                    tb.addBinary((char *)farr1, sizeu * sizeof(float));
                    tb.addBinary((char *)farr2, sizev * sizeof(float));
                    tb.addBinary((char *)farr3, sizew * sizeof(float));
                }
            }
            else if (strcmp(type, "STRGRD") == 0)
            {
                coDoStructuredGrid *sgrid = (coDoStructuredGrid *)co;
                sgrid->getGridSize(&sizeu, &sizev, &sizew);
                sgrid->getAddresses(&farr1, &farr2, &farr3);
                if (cluster)
                {
                    addInt(sizeu);
                    addInt(sizev);
                    addInt(sizew);
                    tb.addBinary((char *)farr1, sizeu * sizev * sizew * sizeof(float));
                    tb.addBinary((char *)farr2, sizeu * sizev * sizew * sizeof(float));
                    tb.addBinary((char *)farr3, sizeu * sizev * sizew * sizeof(float));
                }
            }
            else if (strcmp(type, "POINTS") == 0)
            {
                coDoPoints *points = (coDoPoints *)co;
                size = points->getNumPoints();
                points->getAddresses(&farr1, &farr2, &farr3);
                if (cluster)
                {
                    addInt(size);
                    tb.addBinary((char *)farr1, size * sizeof(float));
                    tb.addBinary((char *)farr2, size * sizeof(float));
                    tb.addBinary((char *)farr3, size * sizeof(float));
                }
            }
            else if (strcmp(type, "SPHERE") == 0)
            {
                coDoSpheres *spheres = (coDoSpheres *)co;
                size = spheres->getNumSpheres();
                geometryFlag = coSphere::RENDER_METHOD_CPU_BILLBOARDS;
                const char *rm = spheres->getAttribute("RENDER_METHOD");
                if (rm)
                {
                    if (!strcmp(rm, "CG_SHADER"))
                        geometryFlag = coSphere::RENDER_METHOD_CG_SHADER;
                    else if (!strcmp(rm, "POINT_SPRITES"))
                        geometryFlag = coSphere::RENDER_METHOD_ARB_POINT_SPRITES;
                    else if (!strcmp(rm, "PARTICLE_CLOUD"))
                        geometryFlag = coSphere::RENDER_METHOD_PARTICLE_CLOUD;
                    else if (!strcmp(rm, "DISC"))
                        geometryFlag = coSphere::RENDER_METHOD_DISC;
                    else if (!strcmp(rm, "TEXTURE"))
                        geometryFlag = coSphere::RENDER_METHOD_TEXTURE;
                    else if (!strcmp(rm, "CG_SHADER_INVERTED"))
                        geometryFlag = coSphere::RENDER_METHOD_CG_SHADER_INVERTED;
                }
                spheres->getAddresses(&farr1, &farr2, &farr3, &farr4);
                if (cluster)
                {
                    addInt(size);
                    addInt(geometryFlag);
                    tb.addBinary((char *)farr1, size * sizeof(float));
                    tb.addBinary((char *)farr2, size * sizeof(float));
                    tb.addBinary((char *)farr3, size * sizeof(float));
                    tb.addBinary((char *)farr4, size * sizeof(float));
                }
            }
            else if (strcmp(type, "LINES") == 0)
            {
                coDoLines *lines = (coDoLines *)co;
                sizeu = lines->getNumLines();
                sizev = lines->getNumVertices();
                size = lines->getNumPoints();
                lines->getAddresses(&farr1, &farr2, &farr3, &iarr1, &iarr2);
                if (cluster)
                {
                    addInt(sizeu);
                    addInt(sizev);
                    addInt(size);
                    tb.addBinary((char *)farr1, size * sizeof(float));
                    tb.addBinary((char *)farr2, size * sizeof(float));
                    tb.addBinary((char *)farr3, size * sizeof(float));
                    tb.addBinary((char *)iarr1, sizev * sizeof(int));
                    tb.addBinary((char *)iarr2, sizeu * sizeof(int));
                }
            }
            else if (strcmp(type, "QUADS") == 0)
            {
                coDoQuads *lines = (coDoQuads *)co;
                sizev = lines->getNumVertices();
                size = lines->getNumPoints();
                lines->getAddresses(&farr1, &farr2, &farr3, &iarr1);
                if (cluster)
                {
                    addInt(sizev);
                    addInt(size);
                    tb.addBinary((char *)farr1, size * sizeof(float));
                    tb.addBinary((char *)farr2, size * sizeof(float));
                    tb.addBinary((char *)farr3, size * sizeof(float));
                    tb.addBinary((char *)iarr1, sizev * sizeof(int));
                }
            }
            else if (strcmp(type, "TRITRI") == 0)
            {
                coDoTriangles *lines = (coDoTriangles *)co;
                sizev = lines->getNumVertices();
                size = lines->getNumPoints();
                lines->getAddresses(&farr1, &farr2, &farr3, &iarr1);
                if (cluster)
                {
                    addInt(sizev);
                    addInt(size);
                    tb.addBinary((char *)farr1, size * sizeof(float));
                    tb.addBinary((char *)farr2, size * sizeof(float));
                    tb.addBinary((char *)farr3, size * sizeof(float));
                    tb.addBinary((char *)iarr1, sizev * sizeof(int));
                }
            }
            else if (strcmp(type, "USTVDT") == 0)
            {
                coDoVec3 *normal_data = (coDoVec3 *)co;
                size = normal_data->getNumPoints();
                normal_data->getAddresses(&farr1, &farr2, &farr3);
                if (cluster)
                {
                    addInt(size);
                    tb.addBinary((char *)farr1, size * sizeof(float));
                    tb.addBinary((char *)farr2, size * sizeof(float));
                    tb.addBinary((char *)farr3, size * sizeof(float));
                }
            }
            else if (strcmp(type, "TEXTUR") == 0)
            {
                coDoTexture *tex = (coDoTexture *)co;
                coDoPixelImage *img = tex->getBuffer();
                texture = (unsigned char *)(img->getPixels());
                sizeu = img->getWidth();
                sizev = img->getHeight();
                sizew = img->getPixelsize();
                numTC = tex->getNumCoordinates();
                textureCoords = tex->getCoordinates();
                if (cluster)
                {
                    addInt(sizeu);
                    addInt(sizev);
                    addInt(sizew);
                    addInt(numTC);
                    tb.addBinary((char *)texture, sizeu * sizev * sizew);
                    tb.addBinary((char *)textureCoords[0], numTC * sizeof(float));
                    tb.addBinary((char *)textureCoords[1], numTC * sizeof(float));
                }
            }
            else if (strcmp(type, "RGBADT") == 0 || strcmp(type, "colors") == 0)
            {
                coDoRGBA *colors = (coDoRGBA *)co;
                colors->getAddress(&pc);
                size = colors->getNumPoints();
                if (cluster)
                {
                    addInt(size);
                    tb.addBinary((char *)pc, size * sizeof(int));
                }
            }
            else if (strcmp(type, "BYTEDT") == 0)
            {
                coDoByte *bytes = (coDoByte *)co;
                bytes->getAddress(&byteData);
                size = bytes->getNumPoints();
                if (cluster)
                {
                    addInt(size);
                    tb.addBinary((const char *)byteData, size);
                }
            }
            else if (strcmp(type, "USTSDT") == 0)
            {
                coDoFloat *volume_sdata = (coDoFloat *)co;
                size = volume_sdata->getNumPoints();
                volume_sdata->getAddress(&farr1);
                if (cluster)
                {
                    addInt(size);
                    tb.addBinary((char *)farr1, size * sizeof(float));
                }
            }
            else
            {
                tb.addBinary("UNKNOW", 6);
                cerr << "unknown dataobject" << endl;
            }
        }
        else
        {
            tb.addBinary("EMPTY ", 6);
            tb << "NoName";
            int nix = 0;
            addInt(nix);
            //send over an empty dataobject
        }

        //std::cerr << "CoviseRenderObject::<init> info: object " << (co ? co->getName() : "*NULL*") << " assigned to slaves";
        //for (std::vector<int>::iterator s = this->assignedTo.begin(); s != this->assignedTo.end(); ++s)
        //   std::cerr << " " << *s;
        //std::cerr << std::endl;

        Message msg(tb);
        coVRMSController::instance()->sendSlaves(&msg);

        //std::cerr << "CoviseRenderObject::<init> info: object " << (co ? co->getName() : "*NULL*") << " sent to slaves" << std::endl;

    } /* endif coVRMSController->isMaster() */
    else // Slave
    {
        // receive a dataobject

        //std::cerr << "CoviseRenderObject::<init> info: read from master" << std::endl;

        Message msg;
        coVRMSController::instance()->readMaster(&msg);
        TokenBuffer tb(&msg);

        strncpy(type, tb.getBinary(7), 7);
        char *n;
        tb >> n;
        name = new char[strlen(n) + 1];
        strcpy(name, n);

        //std::cerr << "CoviseRenderObject::<init> info: object " << (name ? name : "*NULL*") << " assigned to slaves";
        //for (std::vector<int>::iterator s = this->assignedTo.begin(); s != this->assignedTo.end(); ++s)
        //   std::cerr << " " << *s;
        //std::cerr << std::endl;

        copyInt(numAttributes);
        //cerr <<"Receiving Object " << name << " numAttribs: " << numAttributes << endl;
        //cerr <<"type " << type << endl;
        attrNames = new char *[numAttributes];
        attributes = new char *[numAttributes];
        for (i = 0; i < numAttributes; i++)
        {
            tb >> n;
            attrNames[i] = new char[strlen(n) + 1];
            strcpy(attrNames[i], n);
            tb >> n;
            attributes[i] = new char[strlen(n) + 1];
            strcpy(attributes[i], n);
        }

        if (strcmp(type, "GEOMET") == 0)
        {
            copyInt(geometryFlag);
        }

        if (strcmp(type, "SETELE") == 0)
        {
            copyInt(size);
        }
        else if (strcmp(type, "POLYGN") == 0)
        {
            copyInt(sizeu);
            copyInt(sizev);
            copyInt(size);
            farr1 = new float[size];
            farr2 = new float[size];
            farr3 = new float[size];
            iarr1 = new int[sizev];
            iarr2 = new int[sizeu];
            memcpy(farr1, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(farr2, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(farr3, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(iarr1, tb.getBinary(sizev * sizeof(int)), sizev * sizeof(int));
            memcpy(iarr2, tb.getBinary(sizeu * sizeof(int)), sizeu * sizeof(int));
        }
        else if (strcmp(type, "TRIANG") == 0)
        {
            copyInt(sizeu);
            copyInt(sizev);
            copyInt(size);
            farr1 = new float[size];
            farr2 = new float[size];
            farr3 = new float[size];
            iarr1 = new int[sizev];
            iarr2 = new int[sizeu];
            memcpy(farr1, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(farr2, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(farr3, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(iarr1, tb.getBinary(sizev * sizeof(int)), sizev * sizeof(int));
            memcpy(iarr2, tb.getBinary(sizeu * sizeof(int)), sizeu * sizeof(int));
        }
        else if (strcmp(type, "UNIGRD") == 0)
        {
            copyInt(sizeu);
            copyInt(sizev);
            copyInt(sizew);
            copyFloat(minx);
            copyFloat(maxx);
            copyFloat(miny);
            copyFloat(maxy);
            copyFloat(minz);
            copyFloat(maxz);
        }
        else if (strcmp(type, "UNSGRD") == 0)
        {
            copyInt(sizeu);
            copyInt(sizev);
            copyInt(sizew);
            farr1 = new float[sizew];
            farr2 = new float[sizew];
            farr3 = new float[sizew];
            iarr1 = new int[sizev];
            iarr2 = new int[sizeu];
            iarr3 = new int[sizeu];
            memcpy(farr1, tb.getBinary(sizew * sizeof(float)), sizew * sizeof(float));
            memcpy(farr2, tb.getBinary(sizew * sizeof(float)), sizew * sizeof(float));
            memcpy(farr3, tb.getBinary(sizew * sizeof(float)), sizew * sizeof(float));
            memcpy(iarr1, tb.getBinary(sizev * sizeof(int)), sizev * sizeof(int));
            memcpy(iarr2, tb.getBinary(sizeu * sizeof(int)), sizeu * sizeof(int));
            memcpy(iarr3, tb.getBinary(sizeu * sizeof(int)), sizeu * sizeof(int));
        }
        else if (strcmp(type, "RCTGRD") == 0)
        {
            copyInt(sizeu);
            copyInt(sizev);
            copyInt(sizew);
            farr1 = new float[sizeu];
            farr2 = new float[sizev];
            farr3 = new float[sizew];
            memcpy(farr1, tb.getBinary(sizeu * sizeof(float)), sizeu * sizeof(float));
            memcpy(farr2, tb.getBinary(sizev * sizeof(float)), sizev * sizeof(float));
            memcpy(farr3, tb.getBinary(sizew * sizeof(float)), sizew * sizeof(float));
        }
        else if (strcmp(type, "STRGRD") == 0)
        {
            copyInt(sizeu);
            copyInt(sizev);
            copyInt(sizew);
            farr1 = new float[sizeu * sizev * sizew];
            farr2 = new float[sizeu * sizev * sizew];
            farr3 = new float[sizeu * sizev * sizew];
            memcpy(farr1, tb.getBinary(sizeu * sizev * sizew * sizeof(float)), sizeu * sizev * sizew * sizeof(float));
            memcpy(farr2, tb.getBinary(sizeu * sizev * sizew * sizeof(float)), sizeu * sizev * sizew * sizeof(float));
            memcpy(farr3, tb.getBinary(sizeu * sizev * sizew * sizeof(float)), sizeu * sizev * sizew * sizeof(float));
        }
        else if (strcmp(type, "POINTS") == 0)
        {
            copyInt(size);
            farr1 = new float[size];
            farr2 = new float[size];
            farr3 = new float[size];
            memcpy(farr1, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(farr2, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(farr3, tb.getBinary(size * sizeof(float)), size * sizeof(float));
        }
        else if (strcmp(type, "SPHERE") == 0)
        {
            copyInt(size);
            copyInt(geometryFlag);
            farr1 = new float[size];
            farr2 = new float[size];
            farr3 = new float[size];
            farr4 = new float[size];
            memcpy(farr1, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(farr2, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(farr3, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(farr4, tb.getBinary(size * sizeof(float)), size * sizeof(float));
        }
        else if (strcmp(type, "LINES") == 0)
        {
            copyInt(sizeu);
            copyInt(sizev);
            copyInt(size);
            farr1 = new float[size];
            farr2 = new float[size];
            farr3 = new float[size];
            iarr1 = new int[sizev];
            iarr2 = new int[sizeu];
            memcpy(farr1, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(farr2, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(farr3, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(iarr1, tb.getBinary(sizev * sizeof(int)), sizev * sizeof(int));
            memcpy(iarr2, tb.getBinary(sizeu * sizeof(int)), sizeu * sizeof(int));
        }
        else if (strcmp(type, "QUADS") == 0)
        {
            copyInt(sizev);
            copyInt(size);
            farr1 = new float[size];
            farr2 = new float[size];
            farr3 = new float[size];
            iarr1 = new int[sizev];
            memcpy(farr1, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(farr2, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(farr3, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(iarr1, tb.getBinary(sizev * sizeof(int)), sizev * sizeof(int));
        }
        else if (strcmp(type, "TRITRI") == 0)
        {
            copyInt(sizev);
            copyInt(size);
            farr1 = new float[size];
            farr2 = new float[size];
            farr3 = new float[size];
            iarr1 = new int[sizev];
            memcpy(farr1, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(farr2, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(farr3, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(iarr1, tb.getBinary(sizev * sizeof(int)), sizev * sizeof(int));
        }
        else if (strcmp(type, "USTVDT") == 0)
        {
            copyInt(size);
            farr1 = new float[size];
            farr2 = new float[size];
            farr3 = new float[size];
            memcpy(farr1, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(farr2, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(farr3, tb.getBinary(size * sizeof(float)), size * sizeof(float));
        }
        else if (strcmp(type, "STRVDT") == 0)
        {
            copyInt(size);
            farr1 = new float[size];
            farr2 = new float[size];
            farr3 = new float[size];
            memcpy(farr1, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(farr2, tb.getBinary(size * sizeof(float)), size * sizeof(float));
            memcpy(farr3, tb.getBinary(size * sizeof(float)), size * sizeof(float));
        }
        else if (strcmp(type, "TEXTUR") == 0)
        {
            copyInt(sizeu);
            copyInt(sizev);
            copyInt(sizew);
            copyInt(numTC);
            texture = new unsigned char[sizeu * sizev * sizew];
            textureCoords = new float *[2];
            textureCoords[0] = new float[numTC];
            textureCoords[1] = new float[numTC];
            memcpy(texture, tb.getBinary(sizeu * sizev * sizew), sizeu * sizev * sizew);
            memcpy(textureCoords[0], tb.getBinary(numTC * sizeof(float)), numTC * sizeof(float));
            memcpy(textureCoords[1], tb.getBinary(numTC * sizeof(float)), numTC * sizeof(float));
        }
        else if (strcmp(type, "RGBADT") == 0 || strcmp(type, "colors") == 0)
        {
            copyInt(size);
            pc = new int[size];
            memcpy(pc, tb.getBinary(size * sizeof(int)), size * sizeof(int));
        }
        else if (strcmp(type, "BYTEDT") == 0)
        {
            copyInt(size);
            byteData = new unsigned char[size];
            memcpy(byteData, tb.getBinary(size), size);
        }
        else if (strcmp(type, "STRSDT") == 0)
        {
            copyInt(size);
            farr1 = new float[size];
            memcpy(farr1, tb.getBinary(size * sizeof(float)), size * sizeof(float));
        }
        else if (strcmp(type, "USTSDT") == 0)
        {
            copyInt(size);
            farr1 = new float[size];
            memcpy(farr1, tb.getBinary(size * sizeof(float)), size * sizeof(float));
        }
    }
}

CoviseRenderObject::~CoviseRenderObject()
{
    delete[] objs;
    delete geometryObject;
    delete normalObject;
    delete colorObject;
    delete textureObject;
    delete vertexAttributeObject;
    if (!coVRMSController::instance()->isMaster())
    {
        int i;
        delete[] texture;
        if (textureCoords)
        {
            delete[] textureCoords[0];
            delete[] textureCoords[1];
        }
        delete[] textureCoords;
        for (i = 0; i < numAttributes; i++)
        {
            delete[] attrNames[i];
            delete[] attributes[i];
        }
        delete[] attrNames;
        delete[] attributes;
        delete[] farr1;
        delete[] farr2;
        delete[] farr3;
        delete[] farr4;
        delete[] farr5;
        delete[] iarr1;
        delete[] iarr2;
        delete coviseObject;
        delete COVdobj;
        delete COVnormals;
        delete COVcolors;
        delete COVtexture;
        delete COVvertexAttribute;
        delete[] name;
    }
}

CoviseRenderObject *CoviseRenderObject::getGeometry() const
{
    if (geometryObject)
        return geometryObject;
    if (geometryFlag & HASOBJ)
    {
        geometryObject = new CoviseRenderObject(COVdobj, this->assignedTo);
        return geometryObject;
    }
    else
    {
        return NULL;
    }
}

CoviseRenderObject *CoviseRenderObject::getNormals() const
{
    if (normalObject)
        return normalObject;
    if (geometryFlag & HASNORMALS)
    {
        normalObject = new CoviseRenderObject(COVnormals, this->assignedTo);
        return normalObject;
    }
    else
    {
        return NULL;
    }
}

CoviseRenderObject *CoviseRenderObject::getColors() const
{
    if (colorObject)
        return colorObject;
    if (geometryFlag & HASCOLORS)
    {
        colorObject = new CoviseRenderObject(COVcolors, this->assignedTo);
        return colorObject;
    }
    else
    {
        return NULL;
    }
}

CoviseRenderObject *CoviseRenderObject::getTexture() const
{
    if (textureObject)
        return textureObject;
    if (geometryFlag & HASTEXTURE)
    {
        textureObject = new CoviseRenderObject(COVtexture, this->assignedTo);
        return textureObject;
    }
    else
    {
        return NULL;
    }
}

CoviseRenderObject *CoviseRenderObject::getVertexAttribute() const
{
    if (vertexAttributeObject)
        return vertexAttributeObject;
    if (geometryFlag & HASVERTEXATTRIBUTE)
    {
        vertexAttributeObject = new CoviseRenderObject(COVvertexAttribute, this->assignedTo);
        return vertexAttributeObject;
    }
    else
    {
        return NULL;
    }
}

const char *CoviseRenderObject::getAttribute(const char *attributeName) const
{
    for (int i = numAttributes - 1; i >= 0; i--)
    {
        if (strcmp(attributeName, attrNames[i]) == 0)
            return attributes[i];
    }
    return NULL;
}

const char *CoviseRenderObject::getAttributeName(size_t idx) const
{
    if (idx >= numAttributes)
        return NULL;

    return attrNames[idx];
}

const char *CoviseRenderObject::getAttributeValue(size_t idx) const
{
    if (idx >= numAttributes)
        return NULL;

    return attributes[idx];
}

size_t CoviseRenderObject::getNumAttributes() const
{
    return numAttributes;
}

int CoviseRenderObject::getAllAttributes(char **&name, char **&value) const
{
    name = attrNames;
    value = attributes;
    return numAttributes;
}

CoviseRenderObject *CoviseRenderObject::getElement(size_t idx) const
{
    if (idx >= size)
        return NULL;

    int num;
    std::vector<std::vector<int> > assignments;
    RenderObject **objs = getAllElements(num, assignments);

    return dynamic_cast<CoviseRenderObject *>(objs[idx]);
}

RenderObject **CoviseRenderObject::getAllElements(int &numElements, std::vector<std::vector<int> > &assignments) const
{
    numElements = size;
    if (size == 0)
        return NULL;
    int i;

    bool unassigned = assignments.empty();

    if (objs == NULL)
    {
        objs = new RenderObject *[numElements];
        if (coVRMSController::instance()->isMaster())
        {
            const coDoSet *set = (const coDoSet *)coviseObject;
            const coDistributedObject *const *dobjs;
            dobjs = set->getAllElements(&numElements);
            assert(size == numElements);
            for (i = 0; i < numElements; i++)
            {
                //std::cerr << "CoviseRenderObject::getAllElements info: sending " << numElements << " elements";
                if (unassigned)
                {
                    CoviseRenderObject *cobj = new CoviseRenderObject(dobjs[i]);
                    assignments.push_back(cobj->getAssignment());
                    objs[i] = cobj;
                }
                else
                {
                    objs[i] = new CoviseRenderObject(dobjs[i], assignments[i]);
                }
            }
        }
        else
        {
            for (i = 0; i < numElements; i++)
            {
                //std::cerr << "CoviseRenderObject::getAllElements info: receiving " << numElements << " elements";
                if (unassigned)
                {
                    CoviseRenderObject *cobj = new CoviseRenderObject(0);
                    assignments.push_back(cobj->getAssignment());
                    objs[i] = cobj;
                }
                else
                {
                    objs[i] = new CoviseRenderObject(0, assignments[i]);
                }
            }
        }
    }
    return objs;
}

const char *CoviseRenderObject::getType() const
{
    return type;
}

bool
CoviseRenderObject::IsTypeField(const vector<string> &types,
                                bool strict_case)
{
    if (isType("SETELE"))
    {
        int no_elems;
        std::vector<std::vector<int> > a;
        getAllElements(no_elems, a);
        int i;
        bool ret = true;
        for (i = 0; i < no_elems; ++i)
        {
            bool thisType = static_cast<CoviseRenderObject *>(objs[i])->IsTypeField(types, strict_case);
            if (!thisType && strict_case)
            {
                ret = false;
                break;
            }
            else if (!strict_case && thisType)
            {
                break;
            }
        }
        if (!ret) // see !thisType && strict_case
        {
            return false;
        }
        else if (i != no_elems || strict_case) // see !strict_case && thisType
        {
            //   or strict_case and all true
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        unsigned int type;
        for (type = 0; type < types.size(); ++type)
        {
            if (types[type] == getType())
            {
                break;
            }
        }
        if (type == types.size())
        {
            return false;
        }
    }
    return true;
}

int CoviseRenderObject::getFloatRGBA(int pos, float *r, float *g, float *b, float *a) const
{
    if (pos < 0 || pos >= getNumColors())
        return 0;

    unsigned char *chptr = (unsigned char *)&pc[pos];
#ifdef BYTESWAP
    *a = ((float)(*chptr)) / (float)255.0;
    chptr++;
    *b = ((float)(*chptr)) / (float)255.0;
    chptr++;
    *g = ((float)(*chptr)) / (float)255.0;
    chptr++;
    *r = ((float)(*chptr)) / (float)255.0;
#else
    *r = ((float)(*chptr)) / (float)255.0;
    chptr++;
    *g = ((float)(*chptr)) / (float)255.0;
    chptr++;
    *b = ((float)(*chptr)) / (float)255.0;
    chptr++;
    *a = ((float)(*chptr)) / (float)255.0;
#endif
    return 1;
}

const unsigned char *CoviseRenderObject::getByte(Field::Id idx) const
{
    switch (idx)
    {
    case Field::Byte:
        return byteData;
    case Field::Texture:
        return texture;

    case Field::X:
    case Field::Y:
    case Field::Z:
    case Field::Red:
    case Field::Green:
    case Field::Blue:
    case Field::RGBA:
    case Field::Elements:
    case Field::Connections:
    case Field::Types:
        break;
    }

    return NULL;
}

const int *CoviseRenderObject::getInt(Field::Id idx) const
{
    switch (idx)
    {
    case Field::RGBA:
        return pc;
    case Field::Connections:
        return iarr1;
    case Field::Elements:
        return iarr2;
    case Field::Types:
        return iarr3;

    case Field::X:
    case Field::Y:
    case Field::Z:
    case Field::Red:
    case Field::Green:
    case Field::Blue:
    case Field::Byte:
    case Field::Texture:
        break;
    }

    return NULL;
}

const float *CoviseRenderObject::getFloat(Field::Id idx) const
{
    switch (idx)
    {
    case Field::X:
    case Field::Red:
        return farr1;
    case Field::Y:
    case Field::Green:
        return farr2;
    case Field::Z:
    case Field::Blue:
        return farr3;

    case Field::RGBA:
    case Field::Byte:
    case Field::Texture:
    case Field::Elements:
    case Field::Connections:
    case Field::Types:
        break;
    }

    return NULL;
}

bool CoviseRenderObject::isAssignedToMe() const
{

    if (!coVRDistributionManager::instance().isActive() || !coVRMSController::instance()->isCluster())
    {
        return true;
    }

    // Assigned to all
    if (this->assignedTo.size() > 0 && this->assignedTo[0] == -1)
        return true;

    int myID = coVRMSController::instance()->getID();

    bool assignedToMe = find(this->assignedTo.begin(), this->assignedTo.end(), myID) != this->assignedTo.end();

    //   if (assignedToMe)
    //      std::cerr << "CoviseRenderObject::isAssignedToMe info: object " << (getName() ? getName() : "*NULL*") << " is assigned to me" << std::endl;

    return assignedToMe;
}
