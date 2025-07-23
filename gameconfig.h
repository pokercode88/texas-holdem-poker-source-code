#pragma once

#include "Comm/ITableGame.h"
#include "common/macros.h"
#include "common/nndef.h"

using namespace nndef;

namespace game
{
    namespace config
    {
        class GameConfig
        {
            DISALLOW_COPY_AND_ASSIGN(GameConfig)

        private:
            //
            explicit GameConfig();

        public:
            //
            static GameConfig *Create()
            {
                return new GameConfig();
            }

            ~GameConfig();

        public:
            //
            bool loadConfigFile(const char *strPath);
            //
            void printConfigFile(GameInitParam const &sInitParams);
            //
            void loadRoomConfig(const void *p);
            //
            void printRoomConfig();

        public:
            void setRoomKey(std::string value)
            {
                sRoomKey = value;
            }

            std::string getRoomKey()
            {
                return sRoomKey;
            }

            void setRoomName(std::string value)
            {
                sRoomName = value;
            }

            std::string getRoomName()
            {
                return sRoomName;
            }

            void setCreateTime(long value)
            {
                lCreateTime = value;
            }

            long getCreateTime()
            {
                return lCreateTime;
            }

            void setLimitPeople(int value)
            {
                iLimitPeople = value;
            }

            int getLimitPoeple()
            {
                return iLimitPeople;
            }

            void setMinTableBet(long value)
            {
                lMinTableBet = value;
            }

            long getMinTableBet()
            {
                return lMinTableBet;
            }

            void setMaxTableBet(long value)
            {
                lMaxTableBet = value;
            }

            long getMaxTableBet()
            {
                return lMaxTableBet;
            }

            void setMinBet(long value)
            {
                lMinBet = value;
            }

            long getMinBet()
            {
                return lMinBet;
            }

            void setMaxBet(long value)
            {
                lMaxBet = value;
            }

            long getMaxBet()
            {
                return lMaxBet;
            }

            void setFee(long value)
            {
                lFee = value;
            }

            long getFee()
            {
                return lFee;
            }

            void pushBet(long value)
            {
                vBet.push_back(value);
            }

            long popBet(int index)
            {
                if (index >= (int)vBet.size())
                {
                    return 0;
                }

                return vBet[index];
            }
            void clearBet()
            {
                vBet.clear();
            }

            void insertAreaOdds(int key, long value)
            {
                mAreaOdds.insert(std::make_pair(key, value));
            }

            long getAreaOdds(int key)
            {
                auto data = mAreaOdds.find(key);
                if (data != mAreaOdds.end())
                {
                    return data->second;
                }

                return 0;
            }

            void clearAreaOdds()
            {
                mAreaOdds.clear();
            }

            void insertAreaLimit(int key, long value)
            {
                mAreaLimit.insert(std::make_pair(key, value));
            }

            long getAreaLimit(int key)
            {
                auto data = mAreaLimit.find(key);
                if (data != mAreaLimit.end())
                {
                    return data->second;
                }

                return 0;
            }
            void clearAreaLimit()
            {
                mAreaLimit.clear();
            }

            void insertAreaPlayerLimit(int key, long value)
            {
                mAreaPlayerLimit.insert(std::make_pair(key, value));
            }

            long getAreaPlayerLimit(int key)
            {
                auto data = mAreaPlayerLimit.find(key);
                if (data != mAreaPlayerLimit.end())
                {
                    return data->second;
                }

                return 0;
            }
            void clearAreaPlayerLimit()
            {
                mAreaPlayerLimit.clear();
            }

        public:
            //
            int getNNRatioByNNType(E_NN_TYPE nntype);
            //
            inline int getReadyTime()
            {
                return nReadyTime;
            }
            //
            inline int getCardTime()
            {
                return nCardTime ;
            }
            //
            inline int getBetTime()
            {
                return nBetTime;
            }
            //
            inline void setBetTime(long value)
            {
                nBetTime = value;
            }
            inline void setRobotAllAction(const RoomSo::RobotAllAction *pRobotAction)
            {
                if(pRobotAction != nullptr)
                {
                    robotAction = *pRobotAction;
                }
            }
            inline RoomSo::RobotAllAction &getRobotAllAction()
            {
                return robotAction;
            }
            //
            inline int getOpenTime()
            {
                return nOpenTime;
            }
            //
            inline int getDelayTime()
            {
                return nDelayTime;
            }
            //
            inline int getRoomTimeout()
            {
                return nRoomTimeout;
            }
            //
            inline int getEndTime()
            {
                return nEndTime;
            }
            //
            //inline int getMaxBet()
            //{
            //    return nMaxBet;
            //}
            //
            inline int getMaxBanker()
            {
                return nMaxBanker;
            }
            //
            inline int getNotReady()
            {
                return nNotReady;
            }
            //
            inline RoomSo::E_Type getRoomType()
            {
                return eRoomType;
            }
            //
            inline int getPayType()
            {
                return iPayType;
            }
            //
            inline int getMinBegin()
            {
                return nMinBegin;
            }
            //
            inline int getMaxRound()
            {
                return nMaxRound;
            }
            //
            inline int getBaseRatio()
            {
                return nBaseRatio;
            }
            //
            inline int getWinMaxRatio()
            {
                return nWinMaxRatio;
            }
            //
            inline int getBankerPoint()
            {
                return nBankerPoint;
            }
            //
            inline int getMinSit()
            {
                return nMinSit;
            }
            //
            inline int getMinLeft()
            {
                return nMinLeft;
            }
            //
            inline int getFrontBet()
            {
                return iFrontBet;
            }
            //
            inline void setFrontBet(int frontBet)
            {
                iFrontBet = frontBet;
            }
            //
            inline int getBlindNum()
            {
                return iBlindNum <= 0 ? 1 : iBlindNum;
            }
            //
            inline void setBlindNum(int blindNum)
            {
                iBlindNum = blindNum;
            }
            inline int getBlindLevel()
            {
                return iBlindLevel;
            }
            inline void setBlindLevel(int level)
            {
                iBlindLevel = level;
            }
            inline void setMaxSeatNum(int seatNum)
            {
                iMaxSeatNum = seatNum;
            }
            //
            inline int getMinTake()
            {
                return iMinTake;
            }
            inline void setMinTake(long value)
            {
                iMinTake = value;
            }
            //
            inline int getInitPoint()
            {
                return iInitPoint;
            }
            //
            inline int getBasePoint()
            {
                return iBasePoint;
            }
            //
            inline int getProfit()
            {
                return iProfit;
            }
            //
            inline int getNNRatioType()
            {
                return iNNRatioType;
            }
            //
            std::map<int, int> &getMapNNType()
            {
                return mNNType;
            }
            //
            std::vector<E_NN_TYPE> &getVecNNType()
            {
                return vNNType;
            }
            //
            std::map<int, int> &getGuessTypeMap()
            {
                return mGuessType;
            }
            //
            std::vector<int> &getVecGuessNum()
            {
                return vGuessNum;
            }
            //
            inline bool isEnterLimit()
            {
                return bEnterLimit;
            }
            //
            inline bool isRubCard()
            {
                return bRubCard;
            }
            //
            inline bool isBetLimit()
            {
                return bBetLimit;
            }
            //
            inline bool isAddBetLimit()
            {
                return bAddBetLimit;
            }
            //
            inline bool isRaiseBet()
            {
                return bRaiseBet;
            }
            //
            inline bool isKing()
            {
                return bKing;
            }
            //
            inline bool isAddBet()
            {
                return bAddBet;
            }
            //
            inline bool isDoubleBet()
            {
                return bDoubleBet;
            }
            //
            std::vector<int> &getVecUserID()
            {
                return vUserID;
            }
            //
            inline int getRewardInit()
            {
                return nRewardInit;
            }
            //
            inline int getSysInit()
            {
                return nSysInit;
            }
            //
            inline float getSysProfit()
            {
                return nSysProfit;
            }
            //
            inline float getSysRobotProfit()
            {
                return nSysRobotProfit;
            }
            //
            inline long getServiceFee()
            {
                return iServiceFee;
            }
            inline int getMaxSeatNum()
            {
                return iMaxSeatNum;
            }
            inline int getnVersion()
            {
                return nVersion;
            }
            inline int getAiGameRoundLimit()
            {
                return iAiGameRoundLimit;
            }

            inline long getMinSitScore()
            {
                return nMinSitScore;
            }

            long timestamp()
            {
                /* struct timeval tv;
                 gettimeofday(&tv, NULL);
                 return (tv.tv_sec*1000 + tv.tv_usec/1000); */
                return 0;
            }

        public:
            inline long getBetValue(int iIndex)
            {
                if (iIndex <= 0)
                {
                    return 0;
                }

                iIndex--;

                if (iIndex >= (int)vBet.size())
                {
                    return 0;
                }

                return vBet[iIndex];
            }

            inline long getOddsValue(int iAreaID)
            {
                auto iter = mAreaOdds.find(iAreaID);
                if (iter != mAreaOdds.end())
                {
                    return iter->second;
                }

                return 0;
            }

        protected:
            //
            int nReadyTime;
            //
            int nCardTime;
            //
            int nBetTime;
            //
            int nOpenTime;
            //
            int nDelayTime;
            //
            int nEndTime;
            //
            int nRoomTimeout;
            //
            int nNotReady;
            //
            int nMaxBet;
            //
            int nMaxBanker;
            //
            std::map<int, int> mDefaultNNType;
            //
            std::map<int, int> mNNType;
            //
            std::map<int, int> mGuessType;
            //
            std::vector<int> vGuessNum;
            //
            std::vector<E_NN_TYPE> vNNType;
            //
            std::vector<int> vUserID;
            //
            int nRewardInit;
            //
            int nSysInit;
            //
            float nSysProfit;
            //
            float nSysRobotProfit;

            int nVersion;

        protected:
            //最大回合数
            int nMaxRound;
            //
            int nMinBegin;
            //
            int nBaseRatio;
            //
            int nWinMaxRatio;
            //
            int nBankerPoint;
            //
            int nMinSit;
            //
            int nMinLeft;
            //
            int iInitPoint;
            //
            int iBasePoint;
            //
            int iProfit;
            //开局禁入
            bool bEnterLimit;
            //极速玩法
            bool bRubCard;
            //下注限制
            bool bBetLimit;
            //推注限制
            bool bAddBetLimit;
            //闲家买码
            bool bRaiseBet;
            //王癞玩法
            bool bKing;
            //显示推注
            bool bAddBet;
            //加倍玩法
            bool bDoubleBet;
            //服务费用
            long iServiceFee;
            //
            int iNNRatioType;
            //玩法类型
            int iPayType;
            //
            int iFrontBet;
            //盲目数量
            int iBlindNum;
             //盲目等级
            int iBlindLevel;
            //最小携带
            int iMinTake;
            //
            int iMaxSeatNum;
            //
            RoomSo::E_Type eRoomType;
            //
            RoomSo::RobotAllAction robotAction;

            int iAiGameRoundLimit;

            long nMinSitScore;
        protected:
            std::string sRoomKey;                   // 房间KEY
            std::string sRoomName;                  // 房间名称
            long lCreateTime;                       // 创建时间
            int iLimitPeople;                       // 人数限制
            long lMinTableBet;                      // 牌桌最低限额
            long lMaxTableBet;                      // 牌桌最高限额
            long lMinBet;                           // 个人最低限额
            long lMaxBet;                           // 个人最高限额
            long lFee;                              // 服务费(千分比)
            std::vector<long> vBet;                 // 投注额范围(下标就是筹码)
            std::map<int, long> mAreaOdds;          // 区域赔率信息(百分比)
            std::map<int, long> mAreaLimit;         // 区域限额
            std::map<int, long> mAreaPlayerLimit;   // 区域个人限额
        };
    };
};

