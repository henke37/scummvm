package org.scummvm.scummvm;

import java.io.FileOutputStream;
import java.io.IOException;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.PixelFormat;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.pdf.PdfDocument;
import android.graphics.pdf.PdfDocument.PageInfo;
import android.graphics.pdf.PdfDocument.PageInfo.Builder;

import android.print.PageRange;
import android.print.PrintAttributes;
import android.print.PrintDocumentAdapter;
import android.print.PrintDocumentAdapter;
import android.print.PrintDocumentInfo;
import android.print.PrintManager;

import android.os.Bundle;
import android.os.CancellationSignal;
import android.os.ParcelFileDescriptor;

import android.util.Log;

public class PrintJob {
	private long nativePtr;
	
	private PrintManager manager;
	private PrintDocAdapter adapter;
	PrintDocumentAdapter.LayoutResultCallback layoutCallback;
	
	private String title;
	private PrintAttributes printAtts;
	
	private PdfDocument pdf;
	
	private PdfDocument.Page page;
	private Canvas pageCanvas;
	private Paint paint;
	
	
	private int pageNum;
	
	public PrintJob(PrintManager manager, String title, PrintAttributes atts) {
		this.manager=manager;
		adapter=new PrintDocAdapter();
		
		this.title = title;
		printAtts = atts;
		
		Log.d(ScummVM.LOG_TAG, "new PrintJob");
	}
	
	private void print() {
		Log.d(ScummVM.LOG_TAG, "Beginning print");
		manager.print(title, adapter, printAtts);
		
		try {
			synchronized(adapter) {
				while(!adapter.finished) {
					adapter.wait();
				}
			}
		} catch (InterruptedException err) {
			Log.wtf("Print wait got interrupted", err);
		}
		
		Log.d(ScummVM.LOG_TAG, "Ending print");
	}
	
	private void beginPage() {
		Log.d(ScummVM.LOG_TAG, "beginPage");
		
		PrintAttributes.MediaSize size = printAtts.getMediaSize();
		Log.d(ScummVM.LOG_TAG, "init pageInfo");
		PdfDocument.PageInfo.Builder builder = new PdfDocument.PageInfo.Builder(
			size.getWidthMils()*72/1000,
			size.getHeightMils()*72/1000,
			++pageNum
		);
		PdfDocument.PageInfo info = builder.create();
		
		Log.d(ScummVM.LOG_TAG, "pdf.startPage");
		page = pdf.startPage(info);
		pageCanvas = page.getCanvas();
	}
	
	private void endPage() {
		Log.d(ScummVM.LOG_TAG, "endPage");
		pdf.finishPage(page);
		page = null;
		pageCanvas = null;
	}
	
	private void endDoc() {
		Log.d(ScummVM.LOG_TAG, "endDoc");
		
		PrintDocumentInfo.Builder builder = new PrintDocumentInfo.Builder(title+".pdf");
		builder.setPageCount(pageNum);
		layoutCallback.onLayoutFinished(builder.build(), true);
	}
	
	private void abortJob() {
		Log.d(ScummVM.LOG_TAG, "abortJob");
		layoutCallback.onLayoutFailed("Job aborted");
	}
	
	private void drawBitmap(Bitmap bm, Rect dst) {
		Log.d(ScummVM.LOG_TAG, "drawBitmap "+dst.toString());
		
		pageCanvas.drawBitmap(bm, null, dst, null);
	}
	
	private Rect getContentRect() {
		return page.getInfo().getContentRect();
	}
	
	private native void doLayout();

	private class PrintDocAdapter extends PrintDocumentAdapter {
	
		public Boolean finished = false;
	
		public void onStart () {
			Log.d(ScummVM.LOG_TAG, "PrintAdapter Start");
		}
		
		public void onLayout (PrintAttributes oldAttributes, 
                PrintAttributes newAttributes, 
                CancellationSignal cancellationSignal, 
                PrintDocumentAdapter.LayoutResultCallback callback, 
                Bundle extras) {
			Log.d(ScummVM.LOG_TAG, "PrintAdapter onLayout");
			
			if(pdf != null) {
				pdf.close();
			}
			
			printAtts = newAttributes;
			layoutCallback = callback;
			
			pdf = new PdfDocument();
			
			pageNum = 0;
			
			doLayout();
		}
		
		public void onWrite (PageRange[] pages, 
                ParcelFileDescriptor destination, 
                CancellationSignal cancellationSignal, 
                PrintDocumentAdapter.WriteResultCallback callback) {
			Log.d(ScummVM.LOG_TAG, "PrintAdapter onWrite");
			
			try {
				FileOutputStream fileOutputStream = new FileOutputStream(destination.getFileDescriptor());
				
				pdf.writeTo(fileOutputStream);
				fileOutputStream.close();
				pdf.close();
				pdf=null;
				
				callback.onWriteFinished(pages);
			} catch(IOException err) {
				Log.e(ScummVM.LOG_TAG, "PrintJob::onWrite failed", err);
				callback.onWriteFailed(err.getMessage());
			}
		}
		
		public void onFinish () {
			Log.d(ScummVM.LOG_TAG, "PrintAdapter onFinish");
			
			synchronized(this) {
				finished = true;
				this.notifyAll();
			}
			//TODO: set as finished
		}
	}
}