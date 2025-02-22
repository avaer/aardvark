import * as React from 'react';
import  * as ReactDOM from 'react-dom';
import bind from 'bind-decorator';
import { AvGadget,AvOrigin, AvTransform, AvGrabber, AvModel, AvPoker, AvPanelIntersection,
	AvLine,	AvStandardBoxHook } 
	from '@aardvarkxr/aardvark-react';
import { Av, EndpointAddr, EHand, GrabberHighlight, g_builtinModelSphere, EAction, g_builtinModelHead,
	g_builtinModelHandRight, g_builtinModelHandLeft, Permission } from '@aardvarkxr/aardvark-shared'

interface DefaultHandProps
{
	hand: EHand;
}

interface DefaultHandState
{
	grabberHighlight: GrabberHighlight;
	pokerHighlight: boolean;
	currentPanel: EndpointAddr;
}

class DefaultHand extends React.Component< DefaultHandProps, DefaultHandState >
{
	private m_actionListenerHandle = 0;

	constructor( props: any )
	{
		super( props );

		this.state = 
		{ 
			grabberHighlight: GrabberHighlight.None,
			pokerHighlight: false,
			currentPanel: null,
		};

		this.m_actionListenerHandle = AvGadget.instance().listenForActionStateWithComponent( this.props.hand, 
			EAction.B, this );
	}

	@bind updateGrabberHighlight( newHighlight: GrabberHighlight )
	{
		this.setState( { grabberHighlight: newHighlight } );
	}

	@bind updatePokerHighlight( newHighlight: boolean, newPanel: EndpointAddr )
	{
		this.setState( { pokerHighlight: newHighlight, currentPanel: newPanel } );
	}

	componentWillUnmount()
	{
		AvGadget.instance().unlistenForActionState( this.m_actionListenerHandle );
	}
	public render()
	{
		let modelColor = "#222288FF";
		let highlightColor = "#FF0000FF";
		switch( this.state.grabberHighlight )
		{
			case GrabberHighlight.NearHook:
			case GrabberHighlight.Grabbed:
			case GrabberHighlight.WaitingForConfirmation:
			case GrabberHighlight.InRange:
					modelColor = highlightColor;
					break;
		}

		// poker highlight takes priority
		if( this.state.pokerHighlight )
		{
			modelColor = highlightColor;
		}

		let originPath:string;
		let hookName:string;
		let grabberName:string;
		let dropIcon: string;
		switch( this.props.hand )
		{
		case EHand.Left:
			originPath = "/user/hand/left";
			hookName = "left_hand";
			grabberName = "left_hand_grabber";
			dropIcon = g_builtinModelHandLeft;
			break;
		case EHand.Right:
			originPath = "/user/hand/right";
			hookName = "right_hand";
			grabberName = "right_hand_grabber";
			dropIcon = g_builtinModelHandRight;
			break;
		}

		return (
			<AvOrigin path={ originPath }>
				<AvTransform uniformScale= { 0.01 } >
					<AvModel uri={ g_builtinModelSphere } color={ modelColor }/>
				</AvTransform>

				<AvPoker updateHighlight = { this.updatePokerHighlight } >
					{ this.state.pokerHighlight && 
						<>
							<AvPanelIntersection id="panel_highlight" panelId={ this.state.currentPanel }/>
							<AvLine endId="panel_highlight" color="yellow"/>
						</>
					}
				</AvPoker>
				<AvGrabber updateHighlight = { this.updateGrabberHighlight }
					radius={ 0.001 } persistentName={ grabberName }/>
				<AvStandardBoxHook persistentName={ hookName } hand={ this.props.hand }
					dropIconUri={ dropIcon }
					outerVolumeScale={ 2 }
					xMin={-0.15} xMax={0.15}
					yMin={-0.15} yMax={0.25}
					zMin={-0.15} zMax={0.15}
					/>
			</AvOrigin>
		);
	}
}

class DefaultHands extends React.Component< {}, {} >
{
	public render()
	{
		return (
			<>
				<DefaultHand hand={ EHand.Left } />
				<DefaultHand hand={ EHand.Right } />
				<AvOrigin path="/user/head">
					<AvTransform translateY={ 0.2 }>
						<AvStandardBoxHook
							dropIconUri={ g_builtinModelHead }
							xMin={-0.3} xMax={0.3}
							yMin={-0.6} yMax={0.2}
							zMin={-0.3} zMax={0.3}
							outerVolumeScale={ 2.0 }
							persistentName="head"/>
					</AvTransform>
				</AvOrigin>
			</>
		);
	}
}

ReactDOM.render( <DefaultHands/>, document.getElementById( "root" ) );



