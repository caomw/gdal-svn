/* ****************************************************************************
 * Copyright (c) 1998, Frank Warmerdam
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * ****************************************************************************
 *
 * gdalinfo.c
 *
 * Simple application for dumping all the data about a dataset.  Mainly
 * serves as an early test harnass.
 *
 * $Log$
 * Revision 1.11  2000/03/31 13:43:21  warmerda
 * added code to report all corners and gcps
 *
 * Revision 1.10  2000/03/29 15:33:32  warmerda
 * added block size
 *
 * Revision 1.9  2000/03/06 21:50:37  warmerda
 * added min/max support
 *
 * Revision 1.8  2000/03/06 02:18:13  warmerda
 * added overviews, and colour table
 *
 * Revision 1.6  1999/12/30 02:40:17  warmerda
 * Report driver used.
 *
 * Revision 1.5  1999/10/21 13:22:59  warmerda
 * Print band type symbolically rather than numerically.
 *
 * Revision 1.4  1999/10/01 14:45:14  warmerda
 * prettied up
 *
 * Revision 1.3  1999/03/02 21:12:01  warmerda
 * add DMS reporting of lat/long
 *
 * Revision 1.2  1999/01/11 15:27:59  warmerda
 * Add projection support
 *
 * Revision 1.1  1998/12/03 18:35:06  warmerda
 * New
 *
 */

#include "gdal.h"
#include "cpl_string.h"

static int 
GDALInfoReportCorner( GDALDatasetH hDataset, 
                      const char * corner_name,
                      double x, double y );

/************************************************************************/
/*                                main()                                */
/************************************************************************/

int main( int argc, char ** argv )

{
    GDALDatasetH	hDataset;
    GDALRasterBandH	hBand;
    int			i;
    double		adfGeoTransform[6];
    GDALProjDefH	hProjDef;
    GDALDriverH		hDriver;
    char		**papszMetadata;

    if( argc < 2 )
    {
        printf( "Usage: gdalinfo datasetname\n" );
        exit( 10 );
    }

    GDALAllRegister();

    hDataset = GDALOpen( argv[1], GA_ReadOnly );
    
    if( hDataset == NULL )
    {
        fprintf( stderr,
                 "GDALOpen failed - %d\n%s\n",
                 CPLGetLastErrorNo(), CPLGetLastErrorMsg() );
        exit( 1 );
    }
    
/* -------------------------------------------------------------------- */
/*      Report general info.                                            */
/* -------------------------------------------------------------------- */
    hDriver = GDALGetDatasetDriver( hDataset );
    printf( "Driver: %s/%s\n",
            GDALGetDriverShortName( hDriver ),
            GDALGetDriverLongName( hDriver ) );

    printf( "Size is %d, %d\n",
            GDALGetRasterXSize( hDataset ), 
            GDALGetRasterYSize( hDataset ) );

/* -------------------------------------------------------------------- */
/*      Report projection.                                              */
/* -------------------------------------------------------------------- */
    if( GDALGetProjectionRef( hDataset ) != NULL )
    {
        printf( "Projection is `%s'\n",
                GDALGetProjectionRef( hDataset ) );
    }

/* -------------------------------------------------------------------- */
/*      Report Geotransform.                                            */
/* -------------------------------------------------------------------- */
    if( GDALGetGeoTransform( hDataset, adfGeoTransform ) == CE_None )
    {
        printf( "Origin = (%.6f,%.6f)\n",
                adfGeoTransform[0], adfGeoTransform[3] );

        printf( "Pixel Size = (%.6f,%.6f)\n",
                adfGeoTransform[1], adfGeoTransform[5] );
    }

/* -------------------------------------------------------------------- */
/*      Report GCPs.                                                    */
/* -------------------------------------------------------------------- */
    if( GDALGetGCPCount( hDataset ) > 0 )
    {
        printf( "GCP Projection = %s\n", GDALGetGCPProjection(hDataset) );
        for( i = 0; i < GDALGetGCPCount(hDataset); i++ )
        {
            const GDAL_GCP	*psGCP;
            
            psGCP = GDALGetGCPs( hDataset ) + i;

            printf( "GCP[%3d]: Id=%s, Info=%s\n"
                    "          (%g,%g) -> (%g,%g,%g)\n", 
                    i, psGCP->pszId, psGCP->pszInfo, 
                    psGCP->dfGCPPixel, psGCP->dfGCPLine, 
                    psGCP->dfGCPX, psGCP->dfGCPY, psGCP->dfGCPZ );
        }
    }

/* -------------------------------------------------------------------- */
/*      Report metadata.                                                */
/* -------------------------------------------------------------------- */
    papszMetadata = GDALGetDatasetMetadata( hDataset );
    if( CSLCount(papszMetadata) > 0 )
    {
        printf( "Metadata:\n" );
        for( i = 0; papszMetadata[i] != NULL; i++ )
        {
            printf( "  %s\n", papszMetadata[i] );
        }
    }

/* -------------------------------------------------------------------- */
/*      Report corners.                                                 */
/* -------------------------------------------------------------------- */
    printf( "Corner Coordinates:\n" );
    GDALInfoReportCorner( hDataset, "Upper Left", 
                          0.0, 0.0 );
    GDALInfoReportCorner( hDataset, "Lower Left", 
                          0.0, GDALGetRasterYSize(hDataset));
    GDALInfoReportCorner( hDataset, "Upper Right", 
                          GDALGetRasterXSize(hDataset), 0.0 );
    GDALInfoReportCorner( hDataset, "Lower Right", 
                          GDALGetRasterXSize(hDataset), 
                          GDALGetRasterYSize(hDataset) );
    GDALInfoReportCorner( hDataset, "Center", 
                          GDALGetRasterXSize(hDataset)/2.0, 
                          GDALGetRasterYSize(hDataset)/2.0 );

/* ==================================================================== */
/*      Loop over bands.                                                */
/* ==================================================================== */
    for( i = 0; i < GDALGetRasterCount( hDataset ); i++ )
    {
        double      dfMin, dfMax, adfCMinMax[2];
        int         bGotMin, bGotMax;
        int         nBlockXSize, nBlockYSize;

        hBand = GDALGetRasterBand( hDataset, i+1 );
        GDALGetBlockSize( hBand, &nBlockXSize, &nBlockYSize );
        printf( "Band %d Block=%dx%d Type=%s, ColorInterp=%s\n", i+1,
                nBlockXSize, nBlockYSize,
                GDALGetDataTypeName(
                    GDALGetRasterDataType(hBand)),
                GDALGetColorInterpretationName(
                    GDALGetRasterColorInterpretation(hBand)) );

        dfMin = GDALGetRasterMinimum( hBand, &bGotMin );
        dfMax = GDALGetRasterMaximum( hBand, &bGotMax );
        GDALComputeRasterMinMax( hBand, TRUE, adfCMinMax );
        printf( "Min=%.3f/%d, Max=%.3f/%d, Computed Min/Max=%.3f,%.3f\n", 
                dfMin, bGotMin, dfMax, bGotMax, adfCMinMax[0], adfCMinMax[1] );
        
        if( GDALGetOverviewCount(hBand) > 0 )
        {
            int		iOverview;

            printf( "  Overviews: " );
            for( iOverview = 0; 
                 iOverview < GDALGetOverviewCount(hBand);
                 iOverview++ )
            {
                GDALRasterBandH	hOverview;

                if( iOverview != 0 )
                    printf( ", " );

                hOverview = GDALGetOverview( hBand, iOverview );
                printf( "%dx%d", 
                        GDALGetRasterBandXSize( hOverview ),
                        GDALGetRasterBandYSize( hOverview ) );
            }
            printf( "\n" );
        }

        papszMetadata = GDALGetRasterMetadata( hBand );
        if( CSLCount(papszMetadata) > 0 )
        {
            printf( "Metadata:\n" );
            for( i = 0; papszMetadata[i] != NULL; i++ )
            {
                printf( "  %s\n", papszMetadata[i] );
            }
        }

        if( GDALGetRasterColorInterpretation(hBand) == GCI_PaletteIndex )
        {
            GDALColorTableH	hTable;
            int			i;

            hTable = GDALGetRasterColorTable( hBand );
            printf( "  Color Table (%s with %d entries)\n", 
                    GDALGetPaletteInterpretationName(
                        GDALGetPaletteInterpretation( hTable )), 
                    GDALGetColorEntryCount( hTable ) );

            for( i = 0; i < GDALGetColorEntryCount( hTable ); i++ )
            {
                GDALColorEntry	sEntry;

                GDALGetColorEntryAsRGB( hTable, i, &sEntry );
                printf( "  %3d: %d,%d,%d,%d\n", 
                        i, 
                        sEntry.c1,
                        sEntry.c2,
                        sEntry.c3,
                        sEntry.c4 );
            }
        }
    }

    GDALClose( hDataset );
    
    exit( 0 );
}

/************************************************************************/
/*                        GDALInfoReportCorner()                        */
/************************************************************************/

static int 
GDALInfoReportCorner( GDALDatasetH hDataset, 
                      const char * corner_name,
                      double x, double y )

{
    double	dfGeoX, dfGeoY;
    const char  *pszProjection;
    GDALProjDefH hProj;
    double	adfGeoTransform[6];
        
    printf( "%-11s ", corner_name );
    
/* -------------------------------------------------------------------- */
/*      Transform the point into georeferenced coordinates.             */
/* -------------------------------------------------------------------- */
    if( GDALGetGeoTransform( hDataset, adfGeoTransform ) == CE_None )
    {
        pszProjection = GDALGetProjectionRef(hDataset);

        dfGeoX = adfGeoTransform[0] + adfGeoTransform[1] * x
            + adfGeoTransform[2] * y;
        dfGeoY = adfGeoTransform[3] + adfGeoTransform[4] * x
            + adfGeoTransform[5] * y;
    }

    else
    {
        printf( "(%7.1f,%7.1f)\n", x, y );
        return FALSE;
    }

/* -------------------------------------------------------------------- */
/*      Report the georeferenced coordinates.                           */
/* -------------------------------------------------------------------- */
    if( ABS(dfGeoX) < 181 && ABS(dfGeoY) < 91 )
    {
        printf( "(%12.7f,%12.7f) ", dfGeoX, dfGeoY );

    }
    else
    {
        printf( "(%12.3f,%12.3f) ", dfGeoX, dfGeoY );
    }

    if( pszProjection != NULL && strlen(pszProjection) > 0 )
        hProj = GDALCreateProjDef( pszProjection );
    else
        hProj = NULL;

    if( hProj != NULL 
        && GDALReprojectToLongLat( hProj, &dfGeoX, &dfGeoY ) == CE_None )
    {
        
        printf( "(%s,", GDALDecToDMS( dfGeoX, "Long", 2 ) );
        printf( "%s)", GDALDecToDMS( dfGeoY, "Lat", 2 ) );
    }

    printf( "\n" );

    return TRUE;
}

