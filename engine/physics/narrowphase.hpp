#pragma once
#include "contact.hpp"
#include "components/transform.hpp"
#include "components/physics_components.hpp"
#include <glm/glm.hpp>
#include <cmath>
#include <algorithm>
#include <limits>
#include <optional>

namespace narrowphase {

static constexpr int MAX_POLY = 16;

inline glm::vec2 rotate(const glm::vec2& v, float c, float s) {
  return { c * v.x - s * v.y, s * v.x + c * v.y };
}

inline glm::vec2 rotateInv(const glm::vec2& v, float c, float s) {
  return { c * v.x + s * v.y, -s * v.x + c * v.y };
}

inline glm::vec2 worldCenter(const TransformComponent& xf,
                              const glm::vec2& offset) {
  float c = std::cos(xf.rotation), s = std::sin(xf.rotation);
  return xf.position + rotate(offset, c, s);
}

inline int getWorldPoly(glm::vec2* out, const TransformComponent& xf,
                         const BoxCollider* box, const ConvexCollider* convex) {
  float c = std::cos(xf.rotation), s = std::sin(xf.rotation);

  if (box) {
    glm::vec2 center = worldCenter(xf, box->offset);
    glm::vec2 half   = box->halfExtents * xf.scale;
    glm::vec2 ax{c, s}, ay{-s, c};
    out[0] = center - half.x * ax - half.y * ay;
    out[1] = center + half.x * ax - half.y * ay;
    out[2] = center + half.x * ax + half.y * ay;
    out[3] = center - half.x * ax + half.y * ay;
    return 4;
  }

  if (convex && !convex->vertices.empty()) {
    glm::vec2 center = worldCenter(xf, convex->offset);
    int n = std::min(static_cast<int>(convex->vertices.size()), MAX_POLY);
    for (int i = 0; i < n; ++i) {
      glm::vec2 v = convex->vertices[i] * xf.scale;
      out[i] = center + rotate(v, c, s);
    }
    return n;
  }
  return 0;
}

inline glm::vec2 faceNormal(const glm::vec2* v, int n, int i) {
  glm::vec2 edge = v[(i + 1) % n] - v[i];
  float len = glm::length(edge);
  if (len < 1e-8f) return { 0.f, 1.f };
  return { edge.y / len, -edge.x / len };
}

inline glm::vec2 polyCentroid(const glm::vec2* v, int n) {
  glm::vec2 c{ 0.f };
  for (int i = 0; i < n; ++i) c += v[i];
  return c / static_cast<float>(n);
}

inline glm::vec2 worldToLocal(const TransformComponent& xf,
                               const glm::vec2& worldPt) {
  float c = std::cos(xf.rotation), s = std::sin(xf.rotation);
  glm::vec2 d = worldPt - xf.position;
  return rotateInv(d, c, s);
}

inline std::optional<ContactConstraint>
circleVsCircle(entt::entity eA, const TransformComponent& xfA,
               const CircleCollider& cA,
               entt::entity eB, const TransformComponent& xfB,
               const CircleCollider& cB)
{
  glm::vec2 posA = worldCenter(xfA, cA.offset);
  glm::vec2 posB = worldCenter(xfB, cB.offset);
  float rA = cA.radius * std::max(xfA.scale.x, xfA.scale.y);
  float rB = cB.radius * std::max(xfB.scale.x, xfB.scale.y);

  glm::vec2 diff = posB - posA;
  float dist2 = glm::dot(diff, diff);
  float rSum  = rA + rB;
  if (dist2 >= rSum * rSum) return std::nullopt;

  float dist = std::sqrt(dist2);

  ContactConstraint cc;
  cc.bodyA = eA;
  cc.bodyB = eB;
  cc.normal = (dist > 1e-6f) ? diff / dist : glm::vec2{ 0.f, 1.f };
  cc.pointCount = 1;

  auto& pt = cc.points[0];
  pt.position    = posA + cc.normal * rA;
  pt.penetration = rSum - dist;
  pt.localA      = worldToLocal(xfA, pt.position);
  pt.localB      = worldToLocal(xfB, pt.position);
  pt.feature     = { 0, ContactFeature::VERTEX, 0, ContactFeature::VERTEX };

  return cc;
}

inline std::optional<ContactConstraint>
circleVsPoly(entt::entity eCircle, const TransformComponent& xfC,
             const CircleCollider& cc,
             entt::entity ePoly, const TransformComponent& xfP,
             const BoxCollider* box, const ConvexCollider* convex,
             bool flipped)
{
  glm::vec2 polyV[MAX_POLY];
  int nP = getWorldPoly(polyV, xfP, box, convex);
  if (nP < 3) return std::nullopt;

  glm::vec2 center = worldCenter(xfC, cc.offset);
  float radius = cc.radius * std::max(xfC.scale.x, xfC.scale.y);

  float bestSep  = -1e20f;
  int   bestEdge = 0;
  bool  allInside = true;

  for (int i = 0; i < nP; ++i) {
    glm::vec2 n = faceNormal(polyV, nP, i);
    float sep = glm::dot(center - polyV[i], n);
    if (sep > 0.f) allInside = false;
    if (sep > bestSep) {
      bestSep  = sep;
      bestEdge = i;
    }
  }

  if (allInside) {
    glm::vec2 n = faceNormal(polyV, nP, bestEdge);
    ContactConstraint result;
    result.bodyA = flipped ? ePoly   : eCircle;
    result.bodyB = flipped ? eCircle : ePoly;
    result.normal = flipped ? n : -n;
    result.pointCount = 1;

    auto& pt = result.points[0];
    pt.position    = center - n * bestSep;
    pt.penetration = radius - bestSep;
    pt.localA      = worldToLocal(flipped ? xfP : xfC, pt.position);
    pt.localB      = worldToLocal(flipped ? xfC : xfP, pt.position);
    pt.feature     = { static_cast<uint8_t>(bestEdge), ContactFeature::FACE,
                       0, ContactFeature::VERTEX };
    return result;
  }

  float bestDist2 = std::numeric_limits<float>::max();
  glm::vec2 bestPoint{ 0.f };
  int bestIdx = 0;
  ContactFeature::Type bestType = ContactFeature::VERTEX;

  for (int i = 0; i < nP; ++i) {
    int j = (i + 1) % nP;
    glm::vec2 edge = polyV[j] - polyV[i];
    float len2 = glm::dot(edge, edge);
    float t = (len2 > 1e-12f) ? glm::dot(center - polyV[i], edge) / len2 : 0.f;
    t = glm::clamp(t, 0.f, 1.f);
    glm::vec2 cp = polyV[i] + t * edge;
    float d2 = glm::dot(center - cp, center - cp);
    if (d2 < bestDist2) {
      bestDist2 = d2;
      bestPoint = cp;
      if (t < 1e-4f) {
        bestType = ContactFeature::VERTEX;
        bestIdx  = i;
      } else if (t > 1.f - 1e-4f) {
        bestType = ContactFeature::VERTEX;
        bestIdx  = j;
      } else {
        bestType = ContactFeature::FACE;
        bestIdx  = i;
      }
    }
  }

  float dist = std::sqrt(bestDist2);
  if (dist >= radius) return std::nullopt;

  glm::vec2 normal = (dist > 1e-6f)
    ? (center - bestPoint) / dist
    : glm::vec2{ 0.f, 1.f };

  ContactConstraint result;
  result.bodyA  = flipped ? ePoly   : eCircle;
  result.bodyB  = flipped ? eCircle : ePoly;
  result.normal = flipped ? normal : -normal;
  result.pointCount = 1;

  auto& pt = result.points[0];
  pt.position    = bestPoint;
  pt.penetration = radius - dist;
  pt.localA      = worldToLocal(flipped ? xfP : xfC, pt.position);
  pt.localB      = worldToLocal(flipped ? xfC : xfP, pt.position);
  pt.feature     = { static_cast<uint8_t>(bestIdx), bestType,
                     0, ContactFeature::VERTEX };
  return result;
}

namespace detail {

struct ClipVertex {
  glm::vec2 v;
  ContactFeature cf;
};

inline float findAxisLeastPenetration(const glm::vec2* a, int nA,
                                      const glm::vec2* b, int nB,
                                      int& bestFace) {
  float bestSep = -1e20f;
  bestFace = 0;
  for (int i = 0; i < nA; ++i) {
    glm::vec2 n = faceNormal(a, nA, i);
    float minDot = 1e20f;
    for (int j = 0; j < nB; ++j) {
      float d = glm::dot(b[j] - a[i], n);
      if (d < minDot) minDot = d;
    }
    if (minDot > bestSep) {
      bestSep  = minDot;
      bestFace = i;
    }
  }
  return bestSep;
}

inline int findIncidentEdge(const glm::vec2* v, int n,
                             const glm::vec2& refNormal) {
  float minDot = 1e20f;
  int best = 0;
  for (int i = 0; i < n; ++i) {
    float d = glm::dot(faceNormal(v, n, i), refNormal);
    if (d < minDot) {
      minDot = d;
      best   = i;
    }
  }
  return best;
}

inline int clipSegment(ClipVertex out[2], const ClipVertex in[2],
                        const glm::vec2& normal, float offset,
                        uint8_t clipEdge, bool refIsA) {
  int count = 0;
  float d0 = glm::dot(normal, in[0].v) - offset;
  float d1 = glm::dot(normal, in[1].v) - offset;

  if (d0 <= 0.f) out[count++] = in[0];
  if (d1 <= 0.f) out[count++] = in[1];

  if (d0 * d1 < 0.f && count < 2) {
    float t = d0 / (d0 - d1);
    ClipVertex& cv = out[count];
    cv.v = in[0].v + t * (in[1].v - in[0].v);
    if (refIsA) {
      cv.cf.indexA = clipEdge;
      cv.cf.typeA  = ContactFeature::FACE;
      cv.cf.indexB = (d0 > 0.f) ? in[0].cf.indexB : in[1].cf.indexB;
      cv.cf.typeB  = ContactFeature::VERTEX;
    } else {
      cv.cf.indexB = clipEdge;
      cv.cf.typeB  = ContactFeature::FACE;
      cv.cf.indexA = (d0 > 0.f) ? in[0].cf.indexA : in[1].cf.indexA;
      cv.cf.typeA  = ContactFeature::VERTEX;
    }
    ++count;
  }

  return count;
}

}

inline std::optional<ContactConstraint>
polyVsPoly(entt::entity eA, const TransformComponent& xfA,
           const BoxCollider* boxA, const ConvexCollider* convexA,
           entt::entity eB, const TransformComponent& xfB,
           const BoxCollider* boxB, const ConvexCollider* convexB)
{
  glm::vec2 vA[MAX_POLY], vB[MAX_POLY];
  int nA = getWorldPoly(vA, xfA, boxA, convexA);
  int nB = getWorldPoly(vB, xfB, boxB, convexB);
  if (nA < 3 || nB < 3) return std::nullopt;

  int faceA, faceB;
  float sepA = detail::findAxisLeastPenetration(vA, nA, vB, nB, faceA);
  if (sepA > 0.f) return std::nullopt;

  float sepB = detail::findAxisLeastPenetration(vB, nB, vA, nA, faceB);
  if (sepB > 0.f) return std::nullopt;

  const float kRelTol = 0.95f;
  const float kAbsTol = 0.005f; 
  bool useA = sepA >= sepB * kRelTol + kAbsTol;

  const glm::vec2* refV;
  int               refN, refE;
  const glm::vec2* incV;
  int               incN;
  bool              refIsA;

  if (useA) {
    refV = vA; refN = nA; refE = faceA;
    incV = vB; incN = nB;
    refIsA = true;
  } else {
    refV = vB; refN = nB; refE = faceB;
    incV = vA; incN = nA;
    refIsA = false;
  }

  glm::vec2 refNormal = faceNormal(refV, refN, refE);

  int iEdge = detail::findIncidentEdge(incV, incN, refNormal);

  detail::ClipVertex incSeg[2];
  incSeg[0].v = incV[iEdge];
  incSeg[1].v = incV[(iEdge + 1) % incN];

  if (refIsA) {
    incSeg[0].cf = { static_cast<uint8_t>(refE), ContactFeature::FACE,
                     static_cast<uint8_t>(iEdge), ContactFeature::VERTEX };
    incSeg[1].cf = { static_cast<uint8_t>(refE), ContactFeature::FACE,
                     static_cast<uint8_t>((iEdge + 1) % incN), ContactFeature::VERTEX };
  } else {
    incSeg[0].cf = { static_cast<uint8_t>(iEdge), ContactFeature::VERTEX,
                     static_cast<uint8_t>(refE), ContactFeature::FACE };
    incSeg[1].cf = { static_cast<uint8_t>((iEdge + 1) % incN), ContactFeature::VERTEX,
                     static_cast<uint8_t>(refE), ContactFeature::FACE };
  }

  glm::vec2 rv1 = refV[refE];
  glm::vec2 rv2 = refV[(refE + 1) % refN];
  glm::vec2 tangent = glm::normalize(rv2 - rv1);

  uint8_t sideIdx1 = static_cast<uint8_t>(refE);
  uint8_t sideIdx2 = static_cast<uint8_t>((refE + 1) % refN);

  float sideOffset1 = glm::dot(tangent, rv1);
  float sideOffset2 = glm::dot(tangent, rv2);

  detail::ClipVertex clip1[2];
  int n1 = detail::clipSegment(clip1, incSeg, -tangent, -sideOffset1, sideIdx1, refIsA);
  if (n1 < 2) return std::nullopt;

  detail::ClipVertex clip2[2];
  int n2 = detail::clipSegment(clip2, clip1, tangent, sideOffset2, sideIdx2, refIsA);
  if (n2 < 2) return std::nullopt;

  float refFaceOffset = glm::dot(refNormal, rv1);

  glm::vec2 cA = polyCentroid(vA, nA);
  glm::vec2 cB = polyCentroid(vB, nB);
  glm::vec2 dirAtoB = cB - cA;
  glm::vec2 resultNormal = (glm::dot(refNormal, dirAtoB) >= 0.f)
                            ? refNormal : -refNormal;

  ContactConstraint result;
  result.bodyA  = eA;
  result.bodyB  = eB;
  result.normal = resultNormal;
  result.pointCount = 0;

  for (int i = 0; i < n2 && result.pointCount < 2; ++i) {
    float sep = glm::dot(refNormal, clip2[i].v) - refFaceOffset;
    if (sep <= 0.f) {
      auto& pt = result.points[result.pointCount];
      pt.position    = clip2[i].v;
      pt.penetration = -sep;
      pt.localA      = worldToLocal(xfA, pt.position);
      pt.localB      = worldToLocal(xfB, pt.position);
      pt.feature     = clip2[i].cf;
      result.pointCount++;
    }
  }

  if (result.pointCount == 0) return std::nullopt;
  return result;
}

}
