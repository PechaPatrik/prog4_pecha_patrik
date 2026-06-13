#include "CoilyState.h"
#include "CoilyComponent.h"
#include <cstdlib>
#include <climits>

namespace dae
{
    static const int dRow[4] = { -1, -1, 1, 1 };
    static const int dCol[4] = { 0, -1, 1, 0 };

    std::unique_ptr<CoilyBaseState> CoilyEggState::Update(float deltaTime, CoilyComponent& coily)
    {
        const PyramidGrid* grid = coily.GetGrid();

        if (coily.IsAtBottomRow())
        {
            if (m_waitAtBottom > 0.f)
            {
                m_waitingAtBottom = true;
                m_bottomWaitTimer += deltaTime;
                if (m_bottomWaitTimer < m_waitAtBottom)
                    return nullptr;
            }
            return std::make_unique<CoilySnakeState>(m_hopInterval);
        }

        m_groundTimer += deltaTime;
        if (m_groundTimer < m_hopInterval) return nullptr;
        m_groundTimer = 0.f;

        int row = coily.GetGridRow();
        int col = coily.GetGridCol();

        int dir = (rand() % 2) + 2;
        int newRow = row + dRow[dir];
        int newCol = col + dCol[dir];

        bool valid = grid ? grid->IsValid(newRow, newCol) : false;
        if (!valid)
        {
            dir = (dir == 2) ? 3 : 2;
            newRow = row + dRow[dir];
            newCol = col + dCol[dir];
            valid = grid ? grid->IsValid(newRow, newCol) : false;
        }

        if (valid)
            coily.BeginHop(newRow, newCol, m_hopInterval);

        return nullptr;
    }

    std::unique_ptr<CoilyBaseState> CoilyEggState::OnLanded(CoilyComponent&)
    {
        return nullptr;
    }

    int CoilySnakeState::GetCol(bool inAir) const
    {
        int base = 2 + m_direction * 2;
        return inAir ? base + 1 : base;
    }

    std::unique_ptr<CoilyBaseState> CoilySnakeState::Update(float deltaTime, CoilyComponent& coily)
    {
        m_groundTimer += deltaTime;
        if (m_groundTimer < m_hopInterval) return nullptr;
        m_groundTimer = 0.f;

        const PyramidGrid* grid = coily.GetGrid();
        int row = coily.GetGridRow();
        int col = coily.GetGridCol();
        int targetRow = coily.GetTargetRow();
        int targetCol = coily.GetTargetCol();

        int bestDir = -1;
        int bestDist = INT_MAX;
        for (int dir = 0; dir < 4; ++dir)
        {
            int nr = row + dRow[dir];
            int nc = col + dCol[dir];
            if (grid && !grid->IsValid(nr, nc)) continue;
            int dist = abs(nr - targetRow) + abs(nc - targetCol);
            if (dist < bestDist)
            {
                bestDist = dist;
                bestDir = dir;
            }
        }

        if (bestDir == -1) return nullptr;

        m_direction = bestDir;
        coily.BeginHop(row + dRow[bestDir], col + dCol[bestDir], m_hopInterval);

        return nullptr;
    }

    std::unique_ptr<CoilyBaseState> CoilySnakeState::OnLanded(CoilyComponent&)
    {
        return nullptr;
    }
}