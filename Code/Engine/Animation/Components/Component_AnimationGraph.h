#pragma once

#include "Engine/Entity/EntityComponent.h"
#include "Engine/Animation/Graph/Animation_RuntimeGraph_Definition.h"
#include "Engine/Animation/Graph/Animation_RuntimeGraph_Instance.h"
#include "Engine/Animation/Graph/Animation_RuntimeGraph_RootMotionRecorder.h"
#include "Engine/Animation/Graph/Animation_RuntimeGraph_Contexts.h"

//-------------------------------------------------------------------------

namespace EE::Animation
{
    enum class TaskSystemDebugMode;
    enum class RootMotionRecorderDebugMode;

    //-------------------------------------------------------------------------

    class EE_ENGINE_API AnimationGraphComponent final : public EntityComponent
    {
        EE_REGISTER_ENTITY_COMPONENT( AnimationGraphComponent );

        friend class AnimationDebugView;

    public:

        inline AnimationGraphComponent() = default;
        inline AnimationGraphComponent( StringID name ) : EntityComponent( name ) {}
        virtual ~AnimationGraphComponent();

        //-------------------------------------------------------------------------

        inline bool HasGraph() const { return m_pGraphVariation != nullptr; }
        inline bool HasGraphInstance() const { return m_pGraphInstance != nullptr; }
        Skeleton const* GetSkeleton() const;
        Pose const* GetPose() const { return m_pPose; }

        // Does this component require a manual update via a custom entity system?
        inline bool RequiresManualUpdate() const { return m_requiresManualUpdate; }

        // Should we apply the root motion delta automatically to the character once we evaluate the graph 
        // (Note: only works if we dont require a manual update)
        inline bool ShouldApplyRootMotionToEntity() const { return m_applyRootMotionToEntity; }

        // Gets the root motion delta for the last update (Note: this delta is in character space!)
        inline Transform const& GetRootMotionDelta() const { return m_rootMotionDelta; }

        // Get the graph variation ID
        inline ResourceID const& GetGraphVariationID() const { return m_pGraphVariation.GetResourceID(); }

        // This function will change the graph and data-set used! Note: this can only be called for unloaded components
        void SetGraphVariation( ResourceID graphResourceID );

        // Graph evaluation
        //-------------------------------------------------------------------------

        // This function will reset the current graph state
        void ResetGraphState();

        // This function will evaluate the graph and produce the desired root motion delta for the character
        void EvaluateGraph( Seconds deltaTime, Transform const& characterWorldTransform, Physics::Scene* pPhysicsScene );

        // This function will execute all pre-physics tasks - it assumes that the character has already been moved in the scene, so expects the final transform for this frame
        void ExecutePrePhysicsTasks( Transform const& characterWorldTransform );

        // The function will execute the post-physics tasks (if any)
        void ExecutePostPhysicsTasks();

        // Control Parameters
        //-------------------------------------------------------------------------

        template<typename ParameterType>
        void SetControlParameterValue( int16_t parameterIdx, ParameterType const& value )
        {
            EE_ASSERT( m_pGraphInstance != nullptr );
            m_pGraphInstance->SetControlParameterValue( m_graphContext, parameterIdx, value );
        }

        template<typename ParameterType>
        ParameterType GetControlParameterValue( int16_t parameterIdx ) const
        {
            EE_ASSERT( m_pGraphInstance != nullptr );
            return m_pGraphInstance->GetControlParameterValue<ParameterType>( const_cast<GraphContext&>( m_graphContext ), parameterIdx );
        }

        EE_FORCE_INLINE int16_t GetControlParameterIndex( StringID parameterID ) const
        {
            EE_ASSERT( m_pGraphInstance != nullptr );
            return m_pGraphInstance->GetControlParameterIndex( parameterID );
        }

        EE_FORCE_INLINE GraphValueType GetControlParameterValueType( int16_t parameterIdx ) const
        {
            EE_ASSERT( m_pGraphInstance != nullptr );
            return m_pGraphInstance->GetControlParameterType( parameterIdx );
        }

        // Development Interface
        //-------------------------------------------------------------------------

        #if EE_DEVELOPMENT_TOOLS
        inline bool IsNodeActive( int16_t nodeIdx ) const
        {
            EE_ASSERT( m_pGraphInstance != nullptr );
            return m_pGraphInstance->IsNodeActive( const_cast<GraphContext&>( m_graphContext ), nodeIdx );
        }

        inline PoseNodeDebugInfo GetPoseNodeDebugInfo( int16_t nodeIdx ) const
        {
            EE_ASSERT( m_pGraphInstance != nullptr );
            return m_pGraphInstance->GetPoseNodeDebugInfo( const_cast<GraphContext&>( m_graphContext ), nodeIdx );
        }

        template<typename T>
        inline T GetRuntimeNodeValue( int16_t nodeIdx ) const
        {
            EE_ASSERT( m_pGraphInstance != nullptr );
            return m_pGraphInstance->GetRuntimeNodeValue<T>( const_cast<GraphContext&>( m_graphContext ), nodeIdx );
        }

        // Graph debug
        void SetGraphDebugMode( GraphDebugMode mode );
        GraphDebugMode GetGraphDebugMode() const;
        void SetGraphNodeDebugFilterList( TVector<int16_t> const& filterList );

        // Task system debug
        void SetTaskSystemDebugMode( TaskSystemDebugMode mode );
        TaskSystemDebugMode GetTaskSystemDebugMode() const;

        // Root motion debug
        void SetRootMotionDebugMode( RootMotionRecorderDebugMode mode );
        RootMotionRecorderDebugMode GetRootMotionDebugMode() const;

        // Draw all debug visualizations
        void DrawDebug( Drawing::DrawContext& drawingContext );
        #endif

    protected:

        virtual void Initialize() override;
        virtual void Shutdown() override;

    private:

        EE_EXPOSE TResourcePtr<GraphVariation>                 m_pGraphVariation = nullptr;

        GraphInstance*                                          m_pGraphInstance = nullptr;
        TaskSystem*                                             m_pTaskSystem = nullptr;

        Transform                                               m_rootMotionDelta = Transform::Identity;
        GraphContext                                            m_graphContext;
        Pose*                                                   m_pPose = nullptr;
        EE_EXPOSE bool                                         m_requiresManualUpdate = false;  // Does this component require a manual update via a custom entity system?
        EE_EXPOSE bool                                         m_applyRootMotionToEntity = false; // Should we apply the root motion delta automatically to the character once we evaluate the graph. (Note: only works if we dont require a manual update)

        #if EE_DEVELOPMENT_TOOLS
        RootMotionRecorder*                                     m_pRootMotionActionRecorder = nullptr; // Allows nodes to record root motion operations
        #endif
    };
}