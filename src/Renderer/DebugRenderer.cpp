#include "DebugRenderer.h"

DebugRenderer::DebugRenderer(std::shared_ptr<BulletDebugDrawer> bulletDebugDrawer) : m_bulletDebugDrawer(bulletDebugDrawer)
{}

void DebugRenderer::Render(const std::shared_ptr<Camera> &camera)
{
    m_bulletDebugDrawer->Render(camera);
}

void DebugRenderer::DrawTrackCollision(const std::shared_ptr<ONFSTrack> &track)
{
    for (auto &trackblock : track->trackBlocks)
    {
        for (auto &trackEntity : trackblock.track)
        {
            this->DrawAABB(trackEntity.GetAABB());
        }
        for (auto &objectEntity : trackblock.objects)
        {
            this->DrawAABB(objectEntity.GetAABB());
        }
    }
}

void DebugRenderer::DrawAABB(const AABB &aabb)
{
    btVector3 colour = btVector3(0, 0, 0);
    m_bulletDebugDrawer->drawBox(Utils::glmToBullet(aabb.position + aabb.min), Utils::glmToBullet(aabb.position + aabb.max), colour);
}

void DebugRenderer::DrawFrustum(const std::shared_ptr<Camera> &camera)
{
    std::array<glm::vec3, 8> frustumDebugVizPoints = camera->viewFrustum.points;

    btVector3 colour(0, 1, 0);
    // Far Plane
    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[0]), Utils::glmToBullet(frustumDebugVizPoints[1]), colour);
    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[1]), Utils::glmToBullet(frustumDebugVizPoints[2]), colour);
    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[2]), Utils::glmToBullet(frustumDebugVizPoints[3]), colour);
    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[3]), Utils::glmToBullet(frustumDebugVizPoints[0]), colour);

    // Near Plane
    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[4]), Utils::glmToBullet(frustumDebugVizPoints[5]), colour);
    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[5]), Utils::glmToBullet(frustumDebugVizPoints[6]), colour);
    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[6]), Utils::glmToBullet(frustumDebugVizPoints[7]), colour);
    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[7]), Utils::glmToBullet(frustumDebugVizPoints[4]), colour);

    // Near to Far edges
    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[4]), Utils::glmToBullet(frustumDebugVizPoints[0]), colour);
    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[5]), Utils::glmToBullet(frustumDebugVizPoints[1]), colour);
    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[6]), Utils::glmToBullet(frustumDebugVizPoints[2]), colour);
    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(frustumDebugVizPoints[7]), Utils::glmToBullet(frustumDebugVizPoints[3]), colour);
}

void DebugRenderer::DrawCarRaycasts(const std::shared_ptr<Car> &car)
{
    glm::vec3 carBodyPosition = car->carBodyModel.position;
    for (uint8_t rangeIdx = 0; rangeIdx < kNumRangefinders; ++rangeIdx)
    {
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(carBodyPosition), Utils::glmToBullet(car->rangefinderInfo.castPositions[rangeIdx]),
                                      btVector3(2.0f * (kFarDistance - car->rangefinderInfo.rangefinders[rangeIdx]), 2.0f * (car->rangefinderInfo.rangefinders[rangeIdx]), 0));
    }

    // Draw up and down casts
    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(carBodyPosition), Utils::glmToBullet(car->rangefinderInfo.upCastPosition),
                                  btVector3(2.0f * (kFarDistance - car->rangefinderInfo.upDistance), 2.0f * (car->rangefinderInfo.upDistance), 0));
    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(carBodyPosition), Utils::glmToBullet(car->rangefinderInfo.downCastPosition),
                                  btVector3(2.0f * (kFarDistance - car->rangefinderInfo.downDistance), 2.0f * (car->rangefinderInfo.downDistance), 0));
}

void DebugRenderer::DrawVroad(const std::shared_ptr<ONFSTrack> &track)
{
    if (track->tag == NFS_3 || track->tag == NFS_4)
    {
        float vRoadDisplayHeight = 0.2f;
        uint32_t nVroad = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroadHead.nrec;
        for (uint32_t vroadIdx = 0; vroadIdx < nVroad; ++vroadIdx)
        {
            // Render COL Vroad? Should I use TRK VROAD to work across HS too?
            if (vroadIdx < nVroad - 1)
            {
                COLVROAD curVroad = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroad[vroadIdx];
                COLVROAD nextVroad = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroad[vroadIdx + 1];

                INTPT refPt = curVroad.refPt;
                INTPT refPtNext = nextVroad.refPt;

                glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));

                // Transform NFS3/4 coords into ONFS 3d space
                glm::vec3 vroadPoint = rotationMatrix * Utils::FixedToFloat(Utils::PointToVec(refPt)) / NFS3_SCALE_FACTOR;
                glm::vec3 vroadPointNext = rotationMatrix * Utils::FixedToFloat(Utils::PointToVec(refPtNext)) / NFS3_SCALE_FACTOR;

                // Add a little vertical offset so it's not clipping through track geometry
                vroadPoint.y += vRoadDisplayHeight;
                vroadPointNext.y += vRoadDisplayHeight;
                m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPointNext), btVector3(1, 0, 1));
                m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPointNext), btVector3(1, 0, 1));

                glm::vec3 curVroadRightVec = rotationMatrix * Utils::PointToVec(curVroad.right) / 128.f;

                if (Config::get().useFullVroad)
                {
                    glm::vec3 leftWall = ((curVroad.leftWall / 65536.0f) / NFS3_SCALE_FACTOR) * curVroadRightVec;
                    glm::vec3 rightWall = ((curVroad.rightWall / 65536.0f) / NFS3_SCALE_FACTOR) * curVroadRightVec;

                    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPoint - leftWall), btVector3(1, 0, 0.5f));
                    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPoint + rightWall), btVector3(1, 0, 0.5f));
                }
                else
                {
                    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPoint + curVroadRightVec), btVector3(1, 0, 0.5f));
                    m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPoint - curVroadRightVec), btVector3(1, 0, 0.5f));
                }
            }
        }
    }
}

void DebugRenderer::DrawCameraAnimation(const std::shared_ptr<Camera> &camera, const std::shared_ptr<ONFSTrack> &track)
{
    for (uint8_t canIdx = 0; canIdx < track->cameraAnimations.size() - 1; ++canIdx)
    {
        glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));

        // Draw CAN positions
        SHARED::CANPT refPt = track->cameraAnimations[canIdx];
        SHARED::CANPT refPtNext = track->cameraAnimations[canIdx + 1];
        glm::vec3 vroadPoint = rotationMatrix * Utils::FixedToFloat(Utils::PointToVec(refPt)) / NFS3_SCALE_FACTOR;
        glm::vec3 vroadPointNext = rotationMatrix * Utils::FixedToFloat(Utils::PointToVec(refPtNext)) / NFS3_SCALE_FACTOR;
        vroadPoint.y += 0.2f;
        vroadPointNext.y += 0.2f;
        m_bulletDebugDrawer->drawLine(
                Utils::glmToBullet(vroadPoint + camera->initialPosition),
                Utils::glmToBullet(vroadPointNext + camera->initialPosition), btVector3(0, 1, 1));

        // Draw Rotations
        glm::quat RotationMatrix =
                glm::normalize(glm::quat(glm::vec3(glm::radians(0.f), glm::radians(-90.f), 0))) *
                glm::normalize(glm::quat(refPt.od1 / 65536.0f, refPt.od2 / 65536.0f, refPt.od3 / 65536.0f, refPt.od4 / 65536.0f));
        glm::vec3 direction = glm::normalize(vroadPoint * glm::inverse(RotationMatrix));
        m_bulletDebugDrawer->drawLine(Utils::glmToBullet(vroadPoint + camera->initialPosition),
                                      Utils::glmToBullet(vroadPoint + camera->initialPosition + direction), btVector3(0, 0.5, 0.5));
    }
}

