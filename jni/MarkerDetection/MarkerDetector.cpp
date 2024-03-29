/*****************************************************************************
*   MarkerDetector.cpp
*   Example_MarkerBasedAR
******************************************************************************
*   by Khvedchenia Ievgen, 5th Dec 2012
*   http://computer-vision-talks.com
******************************************************************************
*   Ch2 of the book "Mastering OpenCV with Practical Computer Vision Projects"
*   Copyright Packt Publishing 2012.
*   http://www.packtpub.com/cool-projects-with-opencv/book
*****************************************************************************/

////////////////////////////////////////////////////////////////////
// Standard includes:
#include <iostream>
#include <sstream>

////////////////////////////////////////////////////////////////////
// File includes:
#include "MarkerDetector.hpp"
#include "Marker.hpp"
#include "TinyLA.hpp"
#include "DebugHelpers.hpp"

MarkerDetector::MarkerDetector(CameraCalibration calibration)
    : m_minContourLengthAllowed(100)
    , markerSize(100,100)
{
    cv::Mat(3,3, CV_32F, const_cast<float*>(&calibration.getIntrinsic().data[0])).copyTo(camMatrix);
    cv::Mat(4,1, CV_32F, const_cast<float*>(&calibration.getDistorsion().data[0])).copyTo(distCoeff);

    bool centerOrigin = true;
    if (centerOrigin)
    {
        m_markerCorners3d.push_back(cv::Point3f(-0.5f,-0.5f,0));
        m_markerCorners3d.push_back(cv::Point3f(+0.5f,-0.5f,0));
        m_markerCorners3d.push_back(cv::Point3f(+0.5f,+0.5f,0));
        m_markerCorners3d.push_back(cv::Point3f(-0.5f,+0.5f,0));
        /*
        m_markerCorners3d.push_back(cv::Point3f(-0.5f,+0.5f,0));
        m_markerCorners3d.push_back(cv::Point3f(+0.5f,+0.5f,0));
        m_markerCorners3d.push_back(cv::Point3f(+0.5f,-0.5f,0));
        m_markerCorners3d.push_back(cv::Point3f(-0.5f,-0.5f,0));
        */
    }
    else
    {
        m_markerCorners3d.push_back(cv::Point3f(0,0,0));
        m_markerCorners3d.push_back(cv::Point3f(1,0,0));
        m_markerCorners3d.push_back(cv::Point3f(1,1,0));
        m_markerCorners3d.push_back(cv::Point3f(0,1,0));    
    }

    m_markerCorners2d.push_back(cv::Point2f(0,0));
    m_markerCorners2d.push_back(cv::Point2f(markerSize.width-1,0));
    m_markerCorners2d.push_back(cv::Point2f(markerSize.width-1,markerSize.height-1));
    m_markerCorners2d.push_back(cv::Point2f(0,markerSize.height-1));
    /*
    m_markerCorners2d.push_back(cv::Point2f(0,markerSize.height-1));
    m_markerCorners2d.push_back(cv::Point2f(markerSize.width-1,markerSize.height-1));
    m_markerCorners2d.push_back(cv::Point2f(markerSize.width-1,0));
    m_markerCorners2d.push_back(cv::Point2f(0,0));
    */
}

void MarkerDetector::processFrame(const cv::Mat& bgraMat, float scale)
{
    cv::Mat resized;
    cv::resize(bgraMat, resized, cv::Size(), scale, scale);
    std::vector<Marker> markers;
    findMarkers(resized, markers);

    m_transformations.clear();
    m_cameraCenters.clear();
    for (size_t i=0; i<markers.size(); i++) {
        m_transformations.push_back(markers[i].transformation);
        m_cameraCenters.push_back(markers[i].cameraCenter);
    }
}

//void MarkerDetector::processFrame(const BGRAVideoFrame& frame)
//{
//    std::vector<Marker> markers;
//    findMarkers(frame, markers);
//
//    m_transformations.clear();
//    for (size_t i=0; i<markers.size(); i++)
//    {
//        m_transformations.push_back(markers[i].transformation);
//    }
//}

const std::vector<Transformation>& MarkerDetector::getTransformations() const
{
    return m_transformations;
}

const std::vector<cv::Point3f>& MarkerDetector::getCameraCenters() const
{
    return m_cameraCenters;
}

//bool MarkerDetector::findMarkers(const BGRAVideoFrame& frame, std::vector<Marker>& detectedMarkers)
//{
//    cv::Mat bgraMat(frame.height, frame.width, CV_8UC4, frame.data, frame.stride);
//    return this::findMarkers(bgraMat, detectedMarkers);
//}

bool MarkerDetector::findMarkers(const cv::Mat& bgraMat, std::vector<Marker>& detectedMarkers)
{
    // Convert the image to grayscale
    prepareImage(bgraMat, m_grayscaleImage);

    // Make it binary
    performThreshold(m_grayscaleImage, m_thresholdImg);

    // Detect contours
    findContours(m_thresholdImg, m_contours, m_grayscaleImage.cols / 5);

    // debug
    cv::drawContours(bgraMat, m_contours, -1, cv::Scalar(255), 2, CV_AA);

    // Find closed contours that can be approximated with 4 points
    findCandidates(m_contours, detectedMarkers);

    // Find is them are markers
    recognizeMarkers(m_grayscaleImage, detectedMarkers);

    // Fix the coord difference between OpenCV and Unity
    /*
    for (size_t i=0; i<detectedMarkers.size(); i++) {
        Marker& m = detectedMarkers[i];
        for (size_t k=0; k<m.points.size(); k++) {
            m.points[k].y = m_grayscaleImage.rows - m.points[k].y;
        }
    }
    */

    // Calculate their poses
    estimatePosition(detectedMarkers);

    //sort by id
    std::sort(detectedMarkers.begin(), detectedMarkers.end());
    return false;
}

void MarkerDetector::prepareImage(const cv::Mat& bgraMat, cv::Mat& grayscale) const
{
    // Convert to grayscale
    cv::cvtColor(bgraMat, grayscale, CV_BGRA2GRAY);
}

void MarkerDetector::performThreshold(const cv::Mat& grayscale, cv::Mat& thresholdImg) const
{
    cv::threshold(grayscale, thresholdImg, 127, 255, cv::THRESH_BINARY_INV);

    /*
    cv::adaptiveThreshold(grayscale,   // Input image
    thresholdImg,// Result binary image
    255,         // 
    cv::ADAPTIVE_THRESH_GAUSSIAN_C, //
    cv::THRESH_BINARY_INV, //
    7, //
    7  //
    );
    */

#ifdef SHOW_DEBUG_IMAGES
    cv::showAndSave("Threshold image", thresholdImg);
#endif
}

void MarkerDetector::findContours(cv::Mat& thresholdImg, ContoursVector& contours, int minContourPointsAllowed) const
{
    ContoursVector allContours;
    cv::findContours(thresholdImg, allContours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

    contours.clear();
    for (size_t i=0; i<allContours.size(); i++)
    {
        int contourSize = allContours[i].size();
        if (contourSize > minContourPointsAllowed)
        {
            contours.push_back(allContours[i]);
        }
    }

#ifdef SHOW_DEBUG_IMAGES
    {
        cv::Mat contoursImage(thresholdImg.size(), CV_8UC1);
        contoursImage = cv::Scalar(0);
        cv::drawContours(contoursImage, contours, -1, cv::Scalar(255), 2, CV_AA);
        cv::showAndSave("Contours", contoursImage);
    }
#endif
}

void MarkerDetector::findCandidates
(
    const ContoursVector& contours, 
    std::vector<Marker>& detectedMarkers
) 
{
    std::vector<cv::Point>  approxCurve;
    std::vector<Marker>     possibleMarkers;

    // For each contour, analyze if it is a parallelepiped likely to be the marker
    for (size_t i=0; i<contours.size(); i++)
    {
        // Approximate to a polygon
        double eps = contours[i].size() * 0.05;
        cv::approxPolyDP(contours[i], approxCurve, eps, true);

        // We interested only in polygons that contains only four points
        if (approxCurve.size() != 4)
            continue;

        // And they have to be convex
        if (!cv::isContourConvex(approxCurve))
            continue;

        // Ensure that the distance between consecutive points is large enough
        float minDist = std::numeric_limits<float>::max();

        for (int i = 0; i < 4; i++)
        {
            cv::Point side = approxCurve[i] - approxCurve[(i+1)%4];            
            float squaredSideLength = side.dot(side);
            minDist = std::min(minDist, squaredSideLength);
        }

        // Check that distance is not very small
        if (minDist < m_minContourLengthAllowed)
            continue;

        // All tests are passed. Save marker candidate:
        Marker m;

        for (int i = 0; i<4; i++) {
            m.points.push_back( cv::Point2f(approxCurve[i].x,approxCurve[i].y) );
        }

        // Sort the points in anti-clockwise order
        // Trace a line between the first and second point.
        // If the third point is at the right side, then the points are anti-clockwise
        cv::Point v1 = m.points[1] - m.points[0];
        cv::Point v2 = m.points[2] - m.points[0];

        double o = (v1.x * v2.y) - (v1.y * v2.x);

        if (o < 0.0) {
        // if (o > 0.0) {
            //if the third point is in the left side, then sort in anti-clockwise order
            std::swap(m.points[1], m.points[3]);
        }

        possibleMarkers.push_back(m);
    }


    // Remove these elements which corners are too close to each other.  
    // First detect candidates for removal:
    std::vector< std::pair<int,int> > tooNearCandidates;
    for (size_t i=0;i<possibleMarkers.size();i++)
    { 
        const Marker& m1 = possibleMarkers[i];

        //calculate the average distance of each corner to the nearest corner of the other marker candidate
        for (size_t j=i+1;j<possibleMarkers.size();j++)
        {
            const Marker& m2 = possibleMarkers[j];

            float distSquared = 0;

            for (int c = 0; c < 4; c++)
            {
                cv::Point v = m1.points[c] - m2.points[c];
                distSquared += v.dot(v);
            }

            distSquared /= 4;

            if (distSquared < 100)
            {
                tooNearCandidates.push_back(std::pair<int,int>(i,j));
            }
        }				
    }

    // Mark for removal the element of the pair with smaller perimeter
    std::vector<bool> removalMask (possibleMarkers.size(), false);

    for (size_t i=0; i<tooNearCandidates.size(); i++)
    {
        float p1 = perimeter(possibleMarkers[tooNearCandidates[i].first ].points);
        float p2 = perimeter(possibleMarkers[tooNearCandidates[i].second].points);

        size_t removalIndex;
        if (p1 > p2)
            removalIndex = tooNearCandidates[i].second;
        else
            removalIndex = tooNearCandidates[i].first;

        removalMask[removalIndex] = true;
    }

    // Return candidates
    detectedMarkers.clear();
    for (size_t i=0;i<possibleMarkers.size();i++)
    {
        if (!removalMask[i])
            detectedMarkers.push_back(possibleMarkers[i]);
    }
}

void MarkerDetector::recognizeMarkers(const cv::Mat& grayscale, std::vector<Marker>& detectedMarkers)
{
    std::vector<Marker> goodMarkers;

    // Identify the markers
    for (size_t i=0;i<detectedMarkers.size();i++)
    {
        Marker& marker = detectedMarkers[i];

        // Find the perspective transformation that brings current marker to rectangular form
        cv::Mat markerTransform = cv::getPerspectiveTransform(marker.points, m_markerCorners2d);

        // Transform image to get a canonical marker image
        cv::warpPerspective(grayscale, canonicalMarkerImage,  markerTransform, markerSize);

        int nRotations;
        int id = Marker::getMarkerId(canonicalMarkerImage, nRotations);
        if (id !=- 1)
        {
            marker.id = id;
            //sort the points so that they are always in the same order no matter the camera orientation
            std::rotate(marker.points.begin(), marker.points.begin() + 4 - nRotations, marker.points.end());

            goodMarkers.push_back(marker);
        }
    }  

    // Refine marker corners using sub pixel accuracy
    if (goodMarkers.size() > 0)
    {
        std::vector<cv::Point2f> preciseCorners(4 * goodMarkers.size());

        for (size_t i=0; i<goodMarkers.size(); i++)
        {  
            const Marker& marker = goodMarkers[i];      

            for (int c = 0; c <4; c++)
            {
                preciseCorners[i*4 + c] = marker.points[c];
            }
        }

        cv::TermCriteria termCriteria = cv::TermCriteria(cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS, 30, 0.01);
        cv::cornerSubPix(grayscale, preciseCorners, cvSize(5,5), cvSize(-1,-1), termCriteria);

        // Copy refined corners position back to markers
        for (size_t i=0; i<goodMarkers.size(); i++)
        {
            Marker& marker = goodMarkers[i];      

            for (int c=0;c<4;c++) 
            {
                marker.points[c] = preciseCorners[i*4 + c];
            }      
        }
    }

    detectedMarkers = goodMarkers;
}

inline Vector4 calcPlain(Vector3& v1, Vector3& v2, Vector3& v3) {
    Vector3 s;
    s.data[0] = v2.data[0] - v1.data[0];
    s.data[1] = v2.data[1] - v1.data[1];
    s.data[2] = v2.data[2] - v1.data[2];
    // printf("vector s = (%f, %f, %f)\n", s.data[0], s.data[1], s.data[2]);
    Vector3 t;
    t.data[0] = v3.data[0] - v1.data[0];
    t.data[1] = v3.data[1] - v1.data[1];
    t.data[2] = v3.data[2] - v1.data[2];
    // printf("vector t = (%f, %f, %f)\n", t.data[0], t.data[1], t.data[2]);
    Vector3 n = s.crossProduct(t);
    float d = -(n.data[0]*v1.data[0] + n.data[1]*v1.data[1] + n.data[2]*v1.data[2]);
    // printf("d = %f + %f + %f\n", n.data[0]*v1.data[0], n.data[1]*v1.data[1], n.data[2]*v1.data[2]);
    Vector4 res;
    res.data[0] = n.data[0];
    res.data[1] = n.data[1];
    res.data[2] = n.data[2];
    res.data[3] = d;
    return res;
}

void MarkerDetector::estimatePosition(std::vector<Marker>& detectedMarkers)
{
    Vector3 m1;
    m1.data[0] = m_markerCorners3d[0].x;
    m1.data[1] = m_markerCorners3d[0].y;
    m1.data[2] = m_markerCorners3d[0].z;

    Vector3 m2;
    m2.data[0] = m_markerCorners3d[1].x;
    m2.data[1] = m_markerCorners3d[1].y;
    m2.data[2] = m_markerCorners3d[1].z;

    Vector3 m3;
    m3.data[0] = m_markerCorners3d[2].x;
    m3.data[1] = m_markerCorners3d[2].y;
    m3.data[2] = m_markerCorners3d[2].z;

    for (size_t i=0; i<detectedMarkers.size(); i++)
    {
        Marker& m = detectedMarkers[i];
        cv::Mat Rvec;
        cv::Mat_<float> Tvec;
        cv::Mat raux, taux;
        cv::solvePnP(m_markerCorners3d, m.points, camMatrix, distCoeff, raux, taux);
        raux.convertTo(Rvec, CV_32F);
        taux.convertTo(Tvec, CV_32F);

        cv::Mat_<float> rotMat(3,3); 
        cv::Rodrigues(Rvec, rotMat);

        // Copy to transformation matrix
        for (int col=0; col<3; col++)
        {
            for (int row=0; row<3; row++)
            {        
                m.transformation.r().mat[row][col] = rotMat(row, col); // Copy rotation component
            }
            m.transformation.t().data[col] = Tvec(col); // Copy translation component
        }

        float fx = camMatrix.at<float>(0, 0);
        float fy = camMatrix.at<float>(1, 1);
        float cx = camMatrix.at<float>(0, 2);
        float cy = camMatrix.at<float>(1, 2);

        Matrix44 transMat = m.transformation.getMat44();
        cv::Mat_<float> cvMat(4,4);
        for (int r=0; r<4; r++) {
            for (int c=0; c<4; c++) {
                cvMat(c,r) = transMat.mat[r][c];
            }
        }

        for (int i=0; i<4; i++) {
            for (int j=0; j<4; j++) {
                printf("%f ", transMat.mat[i][j]);
            }
            printf("\n");
        }
        Vector3 v1 = transMat*m1;
        Vector3 v2 = transMat*m2;
        Vector3 v3 = transMat*m3;

        printf("v1 => (%f, %f, %f)\n", v1.data[0], v1.data[1], v1.data[2]);
        printf("v2 => (%f, %f, %f)\n", v2.data[0], v2.data[1], v2.data[2]);
        printf("v3 => (%f, %f, %f)\n", v3.data[0], v3.data[1], v3.data[2]);
        Vector4 plainParam = calcPlain(v1, v2, v3);
        printf("plainParam: %f, %f, %f, %f\n", plainParam.data[0], plainParam.data[1], plainParam.data[2], plainParam.data[3]);
        Vector3 p;
        p.data[0] = 0;
        p.data[1] = 0;
        p.data[2] = -plainParam.data[3] / plainParam.data[2];

        // Since solvePnP finds camera location, w.r.t to marker pose, to get marker pose w.r.t to the camera we invert it.
        Matrix44 inv;
        cv::Mat_<float> cvInv = cvMat.inv();
        for (int r=0; r<4; r++) {
            for (int c=0; c<4; c++) {
                inv.mat[r][c] = cvInv(c,r);
            }
        }

        // m.transformation = m.transformation.getInverted();
        // Vector3 center = m.transformation.getMat44()*p;
        Vector3 center = inv*p;
        m.cameraCenter.x = center.data[0];
        m.cameraCenter.y = center.data[1];
        m.cameraCenter.z = center.data[2];
        printf("centerInWorld %f, %f, %f\n", center.data[0], center.data[1], center.data[2]);
    }
}
